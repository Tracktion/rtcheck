#include <dlfcn.h>
#include <stdarg.h>
#include <utility>
#include <bitset>
#include <cassert>
#include <iostream>
#include <numeric>
#include <atomic>
#include <execinfo.h>
#include <regex>

#if __APPLE__
 #include <libkern/OSAtomic.h>
 #include <os/lock.h>
 #include <malloc/malloc.h>
#endif

#if __has_include (<cxxabi.h>)
 #include <cxxabi.h>
#endif

#include "rtcheck.h"
#include "interception.h"

namespace rtc
{
auto to_underlying (auto e)
{
    return static_cast<std::underlying_type_t<decltype(e)>> (e);
}

inline std::string demangle (const std::string& name)
{
   #if __has_include(<cxxabi.h>)
    int status;
    std::unique_ptr<char, decltype (&std::free)> demangled (
        abi::__cxa_demangle (name.c_str(), nullptr, nullptr, &status),
        &std::free);

    if (demangled && status == 0)
        return std::string (demangled.get());
   #endif

    return name;
}

inline std::string extract_symbol (const std::string& line)
{
    // macOS/BSD format: "frame binary address symbol + offset"
    if (line.find ("0x") != std::string::npos)
    {
        std::regex macPattern (R"(^\s*\d+\s+\S+\s+0x[0-9a-fA-F]+\s+(.+?)(?:\s+\+\s+\d+)?$)");
        std::smatch match;

        if (std::regex_match (line, match, macPattern))
            return match[1].str();
    }

    // Linux format: "binary(symbol+offset) [address]"
    {
        size_t lparen = line.find ('(');
        size_t plus = line.find ('+', lparen);

        if (lparen != std::string::npos && plus != std::string::npos)
            return line.substr (lparen + 1, plus - lparen - 1);
    }

    return line;
}


inline std::string get_stacktrace()
{
    std::string result;

    void* stack[128];
    auto frames = backtrace (stack, 128);
    char** frameStrings = backtrace_symbols (stack, frames);

    for (auto i = (decltype (frames)) 0; i < frames; ++i)
        (result.append (i, ' ') += demangle (extract_symbol (frameStrings[i]))) += "\n";

    result += "\n";
    ::free (frameStrings);

    return result;
}


//==============================================================================
//==============================================================================
static bool has_initialised = false;


//==============================================================================
//==============================================================================
/**
    Used to enable intercepting of allocations using new, new[], delete and
    delete[] on the calling thread.
*/
struct realtime_context_state
{
    realtime_context_state() = default;

    /// Enters a real-time context
    void realtime_enter()               { realtime_flag.store (true); }

    /// Exits a real-time context
    void realtime_exit()                { realtime_flag.store (false); }

    /// Returns true if this is in a real-time state
    bool is_realtime_context() const    { return realtime_flag.load(); }

    //==============================================================================
private:
    std::atomic<bool> realtime_flag { false };
};


//==============================================================================
//==============================================================================
#if __APPLE__
struct malloc_zone_wrapper
{
    static void* internal_alloc (size_t size)
    {
        return malloc_zone_malloc (malloc_default_zone(), size);
    }

    static void internal_free (void *ptr)
    {
        malloc_zone_free (malloc_default_zone(), ptr);
    }
};

template<typename Type>
Type& get_thead_local_variable()
{
    static pthread_key_t key;
    static pthread_once_t key_once = PTHREAD_ONCE_INIT;

    auto make_tls_key = []
    {
        [[ maybe_unused ]]auto res = pthread_key_create (&key, malloc_zone_wrapper::internal_free);
        assert(res == 0);
    };

    pthread_once (&key_once, make_tls_key);

    auto *ptr = static_cast<Type*> (pthread_getspecific (key));

    if (ptr == nullptr)
    {
        ptr = static_cast<Type*> (malloc_zone_wrapper::internal_alloc(sizeof (Type)));
        new(ptr) Type();
        pthread_setspecific (key, ptr);
    }

    return *ptr;
}

realtime_context_state& get_realtime_context_state()
{
    return get_thead_local_variable<realtime_context_state>();
}

std::bitset<64>& get_disabled_flags_for_thread()
{
    return get_thead_local_variable<std::bitset<64>>();
}
#else
realtime_context_state& get_realtime_context_state()
{
    thread_local realtime_context_state rcs;
    return rcs;
}

std::bitset<64>& get_disabled_flags_for_thread()
{
    thread_local std::bitset<64> disabled_flags;
    return disabled_flags;
}
#endif

realtime_context::realtime_context()
{
    get_realtime_context_state().realtime_enter();
}

realtime_context::~realtime_context()
{
    get_realtime_context_state().realtime_exit();
}

non_realtime_context::non_realtime_context()
{
    assert (get_realtime_context_state().is_realtime_context());
    get_realtime_context_state().realtime_exit();
}

non_realtime_context::~non_realtime_context()
{
    get_realtime_context_state().realtime_enter();
}

bool is_real_time_context()
{
    return get_realtime_context_state().is_realtime_context();
}

void log_function_if_realtime_context (const char* function_name)
{
    if (! has_initialised)
        return;

    if (! is_real_time_context())
        return;

    non_realtime_context nrc;

    std::string_view name (function_name), wrap_prefix ("wrap_");

    if (name.starts_with (wrap_prefix))
        name = name.substr (wrap_prefix.length());

    std::cerr << "Real-time violation: intercepted call to real-time unsafe function " << name << " in real-time context! Stack trace:\n" << get_stacktrace() << std::endl;

    if (get_error_mode() == error_mode::exit)
        std::exit (1);
}

//==============================================================================
void disable_checks_for_thread (uint64_t flags)
{
    get_disabled_flags_for_thread() = flags;
}

void disable_checks_for_thread (check_flags flags)
{
    disable_checks_for_thread(static_cast<uint64_t> (flags));
}

bool are_all_bits_enabled (uint64_t flags, std::bitset<64> disabled_bits)
{
    assert(static_cast<uint64_t> (flags) <= std::numeric_limits<uint64_t>::max());
    const std::bitset<64> bits (flags);
    const auto disabled_bits_interested_in = disabled_bits & bits;

    return (bits ^ disabled_bits_interested_in) == bits;
}

#ifndef NDEBUG
struct check_flags_tests
{
    check_flags_tests()
    {
        assert(! are_all_bits_enabled (to_underlying (check_flags::malloc), 0b1));
        assert(! are_all_bits_enabled (to_underlying (check_flags::malloc) + to_underlying (check_flags::realloc), 0b101));
        assert(are_all_bits_enabled (to_underlying (check_flags::malloc) + to_underlying (check_flags::calloc), 0b100));
        assert(are_all_bits_enabled (to_underlying (check_flags::syscall) + to_underlying (check_flags::openat), 0b0));
        auto a1 = to_underlying (check_flags::syscall);
        auto a2 = to_underlying (check_flags::openat);
        assert(! are_all_bits_enabled (to_underlying (check_flags::syscall) + to_underlying (check_flags::openat), 0b100010000000000000000000000000000000000));
    }
};

static check_flags_tests check_flags_tests;
#endif

bool is_check_enabled_for_thread (check_flags check)
{
    assert(std::bitset<64> (static_cast<uint64_t> (check)).count() == 1 && "Only one flag can be check with this function");
    return are_all_bits_enabled (static_cast<uint64_t> (check), get_disabled_flags_for_thread());
}


//==============================================================================
// details
//==============================================================================
std::atomic<error_mode>& get_error_mode_flag()
{
    static std::atomic<error_mode> em { error_mode::exit };
    return em;
}

void set_error_mode (error_mode em)
{
    get_error_mode_flag().store (em, std::memory_order_release);
}

error_mode get_error_mode()
{
    return get_error_mode_flag().load (std::memory_order_acquire);
}
}

void log_function_if_realtime_context_and_enabled (rtc::check_flags flag, const char* function_name)
{
    if (! rtc::has_initialised)
        return;

    if (rtc::is_check_enabled_for_thread (flag))
        rtc::log_function_if_realtime_context (function_name);
}


//==============================================================================
// memory
//==============================================================================
INTERCEPTOR(void*, malloc, size_t size)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::malloc, __func__);
    INTERCEPT_FUNCTION(void*, malloc, size_t);

    return REAL(malloc)(size);
}


INTERCEPTOR(void*, calloc, size_t size, size_t item_size)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::calloc, __func__);

    INTERCEPT_FUNCTION(void*, calloc, size_t, size_t);
    return REAL(calloc)(size, item_size);
}

INTERCEPTOR(void*, realloc, void *ptr, size_t new_size)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::realloc, __func__);

    INTERCEPT_FUNCTION(void*, realloc, void*, size_t);
    return REAL(realloc)(ptr, new_size);
}

#ifdef __APPLE__
INTERCEPTOR(void *, reallocf, void *ptr, size_t size)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::reallocf, __func__);

    INTERCEPT_FUNCTION(void*, reallocf, void*, size_t);
    return REAL(reallocf)(ptr, size);
}
#endif

INTERCEPTOR(void*, valloc, size_t size)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::valloc, __func__);

    INTERCEPT_FUNCTION(void*, valloc, size_t);
    return REAL(valloc)(size);
}

INTERCEPTOR(void, free, void* ptr)
{
    if (ptr != nullptr)
        log_function_if_realtime_context_and_enabled (rtc::check_flags::free, __func__);

    INTERCEPT_FUNCTION(void, free, void*);
    return REAL(free)(ptr);
}

INTERCEPTOR(int, posix_memalign, void **memptr, size_t alignment, size_t size)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::posix_memalign, __func__);

    INTERCEPT_FUNCTION(int, posix_memalign, void**, size_t, size_t);
    return REAL(posix_memalign)(memptr, alignment, size);
}

INTERCEPTOR(void *, mmap, void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::mmap, __func__);

    INTERCEPT_FUNCTION(void*, mmap, void*, size_t, int, int, int, off_t);
    return REAL(mmap)(addr, length, prot, flags, fd, offset);
}

INTERCEPTOR(int, munmap, void* addr, size_t length)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::munmap, __func__);

    INTERCEPT_FUNCTION(int, munmap, void*, size_t);
    return REAL(munmap)(addr, length);
}


//==============================================================================
// threads
//==============================================================================
INTERCEPTOR(int, pthread_create, pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_create, __func__);
    INTERCEPT_FUNCTION(int, pthread_create, pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
    return REAL(pthread_create)(thread, attr, start_routine, arg);
}

INTERCEPTOR(int, pthread_mutex_lock, pthread_mutex_t *mutex)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_mutex_lock, __func__);

    INTERCEPT_FUNCTION(int, pthread_mutex_lock, pthread_mutex_t*);
    return REAL(pthread_mutex_lock)(mutex);
}

INTERCEPTOR(int, pthread_mutex_unlock, pthread_mutex_t *mutex)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_mutex_unlock, __func__);

    INTERCEPT_FUNCTION(int, pthread_mutex_unlock, pthread_mutex_t*);
    return REAL(pthread_mutex_unlock)(mutex);
}

INTERCEPTOR(int, pthread_join, pthread_t thread, void **value_ptr)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_join, __func__);

    INTERCEPT_FUNCTION(int, pthread_join, pthread_t, void **);
    return REAL(pthread_join)(thread, value_ptr);
}

INTERCEPTOR(int, pthread_cond_signal, pthread_cond_t *cond)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_cond_signal, __func__);

    INTERCEPT_FUNCTION(int, pthread_cond_signal, pthread_cond_t *);
    return REAL(pthread_cond_signal)(cond);
}

INTERCEPTOR(int, pthread_cond_broadcast, pthread_cond_t *cond)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_cond_broadcast, __func__);

    INTERCEPT_FUNCTION(int, pthread_cond_broadcast, pthread_cond_t *);
    return REAL(pthread_cond_broadcast)(cond);
}

INTERCEPTOR(int, pthread_cond_wait, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_cond_wait, __func__);

    INTERCEPT_FUNCTION(int, pthread_cond_wait, pthread_cond_t *, pthread_mutex_t *);
    return REAL(pthread_cond_wait)(cond, mutex);
}

INTERCEPTOR(int, pthread_rwlock_init, pthread_rwlock_t *rwlock,
            const pthread_rwlockattr_t *attr)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_rwlock_init, __func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_init, pthread_rwlock_t *, const pthread_rwlockattr_t *);
    return REAL(pthread_rwlock_init)(rwlock, attr);
}

INTERCEPTOR(int, pthread_rwlock_destroy, pthread_rwlock_t *rwlock)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_rwlock_destroy, __func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_destroy, pthread_rwlock_t *);
    return REAL(pthread_rwlock_destroy)(rwlock);
}

INTERCEPTOR(int, pthread_cond_timedwait, pthread_cond_t *cond,
            pthread_mutex_t *mutex, const timespec *ts)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_cond_timedwait, __func__);

    INTERCEPT_FUNCTION(int, pthread_cond_timedwait, pthread_cond_t *,
                        pthread_mutex_t *, const timespec *);
    return REAL(pthread_cond_timedwait)(cond, mutex, ts);
}

INTERCEPTOR(int, pthread_rwlock_rdlock, pthread_rwlock_t *lock)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_rwlock_rdlock, __func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_rdlock, pthread_rwlock_t *);
    return REAL(pthread_rwlock_rdlock)(lock);
}

INTERCEPTOR(int, pthread_rwlock_unlock, pthread_rwlock_t *lock)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_rwlock_unlock, __func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_unlock, pthread_rwlock_t *);
    return REAL(pthread_rwlock_unlock)(lock);
}

INTERCEPTOR(int, pthread_rwlock_wrlock, pthread_rwlock_t *lock)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_rwlock_wrlock, __func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_wrlock, pthread_rwlock_t *);
    return REAL(pthread_rwlock_wrlock)(lock);
}


#ifndef __APPLE__
INTERCEPTOR(int, pthread_spin_lock, pthread_spinlock_t *spinlock)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::pthread_spin_lock, __func__);
    INTERCEPT_FUNCTION(int, pthread_spin_lock, pthread_spinlock_t*);
    return REAL(pthread_spin_lock)(spinlock);
}

INTERCEPTOR(int, futex, int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::futex, __func__);

    INTERCEPT_FUNCTION(int, futex, int*, int, int, const struct timespec*, int*, int);
    return REAL(futex)(uaddr, op, val, timeout, uaddr2, val3);
}
#endif

//==============================================================================
// sleep
//==============================================================================
INTERCEPTOR(unsigned int, sleep, unsigned int seconds)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::sleep, __func__);

    static auto real = (unsigned int (*)(unsigned int))dlsym(RTLD_NEXT, "sleep");
    return REAL(sleep)(seconds);
}

INTERCEPTOR(int, usleep, useconds_t useconds)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::usleep, __func__);

    INTERCEPT_FUNCTION(int, usleep, useconds_t);
    return REAL(usleep)(useconds);
}

INTERCEPTOR(int, nanosleep, const struct timespec *req, struct timespec * rem)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::nanosleep, __func__);

    INTERCEPT_FUNCTION(int, nanosleep, const struct timespec *, struct timespec *);
    return REAL(nanosleep)(req, rem);
}

//==============================================================================
// files
//==============================================================================
INTERCEPTOR(int, stat, const char* pathname, struct stat* statbuf)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::stat, __func__);

    INTERCEPT_FUNCTION(int, stat, const char*, struct stat*);
    return REAL(stat)(pathname, statbuf);
}

INTERCEPTOR(int, fstat, int fd, struct stat *statbuf)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::fstat, __func__);

    INTERCEPT_FUNCTION(int, fstat, int, struct stat*);
    return REAL(fstat)(fd, statbuf);
}

INTERCEPTOR(int, open, const char *path, int oflag, ...)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::open, __func__);

    INTERCEPT_FUNCTION(int, open, const char*, int, ...);

    va_list args;
    va_start(args, oflag);
    const mode_t mode = va_arg(args, int);
    va_end(args);

    return REAL(open)(path, oflag, mode);
}

INTERCEPTOR(FILE*, fopen, const char *path, const char *mode)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::fopen, __func__);

    INTERCEPT_FUNCTION(FILE*, fopen, const char*, const char*);
    auto result = REAL(fopen)(path, mode);

    return result;
}

INTERCEPTOR(int, openat, int fd, const char *path, int oflag, ...)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::openat, __func__);

    INTERCEPT_FUNCTION(int, openat, int, const char*, int, ...);

    va_list args;
    va_start(args, oflag);
    mode_t mode = va_arg(args, int);
    va_end(args);

    return REAL(openat)(fd, path, oflag, mode);
}

INTERCEPTOR(int, fcntl, int filedes, int cmd, ...)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::fcntl, __func__);

    INTERCEPT_FUNCTION(int, fcntl, int, int, ...);

    va_list args;
    va_start(args, cmd);

    // From RTsan:
    // Following precedent here. The linux source (fcntl.c, do_fcntl) accepts the
    // final argument in a variable that will hold the largest of the possible
    // argument types (pointers and ints are typical in fcntl) It is then assumed
    // that the implementation of fcntl will cast it properly depending on cmd.
    //
    // This is also similar to what is done in
    // sanitizer_common/sanitizer_common_syscalls.inc
    const unsigned long arg = va_arg(args, unsigned long);
    int result = REAL(fcntl)(filedes, cmd, arg);

    va_end(args);

    return result;
}

//==============================================================================
// system
//==============================================================================
#ifndef __APPLE__
INTERCEPTOR(long, schedule, void)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::schedule, __func__);

    INTERCEPT_FUNCTION(long, schedule, void);
    return REAL(schedule)();
}

INTERCEPTOR(long, context_switch, struct task_struct *prev, struct task_struct *next)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::context_switch, __func__);

    INTERCEPT_FUNCTION(long, context_switch, struct task_struct *, struct task_struct *);
    return REAL(context_switch)(prev, next);
}
#endif

#pragma clang diagnostic push
// syscall is deprecated, but still in use in libc++
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define FORWARD_ARGS(func, ...) func(__VA_ARGS__)
#define EXPAND_ARGS(...) (__VA_ARGS__)

INTERCEPTOR(long int, syscall, long int sid, ...)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::syscall, __func__);

    INTERCEPT_FUNCTION(long, syscall, long, ...);

    va_list args;
    va_start(args, sid);
    va_end(args);

    return REAL(syscall)(sid);
}

#pragma clang diagnostic pop


//==============================================================================
// Apple
//==============================================================================
#if __APPLE__

#pragma clang diagnostic push
// OSSpinLockLock is deprecated, but still in use in libc++
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

INTERCEPTOR(void, OSSpinLockLock, volatile OSSpinLock *lock)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::OSSpinLockLock, __func__);
    return REAL(OSSpinLockLock)(lock);
}

INTERCEPTOR(void, os_unfair_lock_lock, os_unfair_lock_t lock)
{
    log_function_if_realtime_context_and_enabled (rtc::check_flags::os_unfair_lock_lock, __func__);
    return REAL(os_unfair_lock_lock)(lock);
}

#pragma clang diagnostic pop

// Newer macOS versions use an internal _os_nospin_lock_lock which is interecpted as per LLVM's RTSan in this commit:
// https://code.ornl.gov/llvm-doe/llvm-project/-/commit/481a55a3d9645a6bc1540d326319b78ad8ed8db1
extern "C" {
    // A pointer to this type is in the interface for `_os_nospin_lock_lock`, but
    // it's an internal implementation detail of `os/lock.c` on Darwin, and
    // therefore not available in any headers. As a workaround, we forward declare
    // it here, which is enough to facilitate interception of _os_nospin_lock_lock.
    struct _os_nospin_lock_s;
    using _os_nospin_lock_t = _os_nospin_lock_s *;
}

INTERCEPTOR(void, _os_nospin_lock_lock, _os_nospin_lock_t lock) {
    log_function_if_realtime_context_and_enabled (rtc::check_flags::os_unfair_lock_lock, __func__);
    return REAL(_os_nospin_lock_lock)(lock);
}

#endif

//==============================================================================
// init
//==============================================================================
__attribute__((constructor))
void init()
{
    rtc::has_initialised = true;
}
