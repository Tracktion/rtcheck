#pragma once

#include <memory>
#include <atomic>
#include <cassert>
#include <string>
#include <iostream>
#include <mutex>
#include <execinfo.h>

#if __has_include (<cxxabi.h>)
 #include <cxxabi.h>
#endif

namespace rt_check
{
    enum class error_mode
    {
        exit,   /// Exit with value 1, default
        cont    /// Continue execution
    };

    void set_error_mode (error_mode);
    error_mode get_error_mode();
}

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
/** Returns a realtime_context_state for the current thread. */
realtime_context_state& get_realtime_context_state();

//==============================================================================
inline bool is_real_time_context()
{
    return get_realtime_context_state().is_realtime_context();
}

//==============================================================================
struct realtime_context
{
    realtime_context()
    {
        get_realtime_context_state().realtime_enter();
    }

    ~realtime_context()
    {
        get_realtime_context_state().realtime_exit();
    }
};

//==============================================================================
struct non_realtime_context
{
    non_realtime_context()
    {
        assert (get_realtime_context_state().is_realtime_context());
        get_realtime_context_state().realtime_exit();
    }

    ~non_realtime_context()
    {
        get_realtime_context_state().realtime_enter();
    }
};


//==============================================================================
//==============================================================================
void log_function_if_realtime_context (const char* function_name);


//==============================================================================
//==============================================================================
enum class check_flags : uint64_t
{
    //==============================================================================
    // memory
    //==============================================================================
    malloc                          = 1 << 0,
    calloc                          = 1 << 1,
    realloc                         = 1 << 2,
    reallocf                        = 1 << 3, // apple only
    valloc                          = 1 << 4,
    free                            = 1 << 5,
    posix_memalign                  = 1 << 6,
    mmap                            = 1 << 7,
    munmap                          = 1 << 8,

    memory                          = malloc | calloc | realloc | reallocf | valloc
                                        | free | posix_memalign | mmap | munmap,

    //==============================================================================
    // threads
    //==============================================================================
    pthread_create                  = 1 << 9,
    pthread_mutex_lock              = 1 << 10,
    pthread_mutex_unlock            = 1 << 11,
    pthread_join                    = 1 << 12,
    pthread_cond_signal             = 1 << 13,
    pthread_cond_broadcast          = 1 << 14,
    pthread_cond_wait               = 1 << 15,
    pthread_rwlock_init             = 1 << 16,
    pthread_rwlock_destroy          = 1 << 17,
    pthread_cond_timedwait          = 1 << 18,
    pthread_rwlock_rdlock           = 1 << 19,
    pthread_rwlock_unlock           = 1 << 20,
    pthread_rwlock_wrlock           = 1 << 21,
    pthread_spin_lock               = 1 << 22,      // linux only
    futex                           = 1 << 23,      // linux only
    OSSpinLockLock                  = 1ull << 35,   // apple only
    os_unfair_lock_lock             = 1ull << 36,   // apple only

    threads = pthread_create | pthread_mutex_lock | pthread_mutex_unlock | pthread_join
                | pthread_cond_signal | pthread_cond_broadcast | pthread_cond_wait
                | pthread_rwlock_init | pthread_rwlock_destroy | pthread_cond_timedwait
                | pthread_rwlock_rdlock | pthread_rwlock_unlock | pthread_rwlock_wrlock
                | pthread_spin_lock | futex | OSSpinLockLock | os_unfair_lock_lock,

    //==============================================================================
    // sleeping
    //==============================================================================
    sleep                           = 1 << 24,
    usleep                          = 1 << 25,
    nanosleep                       = 1 << 26,

    sleeping                        = sleep | usleep | nanosleep,

    //==============================================================================
    // files
    //==============================================================================
    stat                            = 1 << 27,
    fstat                           = 1 << 28,
    open                            = 1 << 29,
    fopen                           = 1 << 30,
    openat                          = 1ull << 31,

    files                           = stat | fstat | open | fopen | openat,

    //==============================================================================
    // system
    //==============================================================================
    schedule                        = 1ull << 32,  // linux only
    context_switch                  = 1ull << 33,  // linux only
    syscall                         = 1ull << 34,

    sys                             = schedule | context_switch | syscall
};

void disable_checks_for_thread (uint64_t flags);
void disable_checks_for_thread (check_flags);

[[nodiscard]] bool is_check_enabled_for_thread (check_flags);

//==============================================================================
//==============================================================================
inline std::string demangle (std::string name)
{
   #if __has_include (<cxxabi.h>)
    int status;

    if (char* demangled = abi::__cxa_demangle (name.c_str(), nullptr, nullptr, &status); status == 0)
    {
        std::string demangledString (demangled);
        free (demangled);
        return demangledString;
    }
   #endif

    return name;
}

inline std::string get_stacktrace()
{
    std::string result;

    void* stack[128];
    auto frames = backtrace (stack, 128);
    char** frameStrings = backtrace_symbols (stack, frames);

    for (auto i = (decltype (frames)) 0; i < frames; ++i)
        (result.append (i, ' ') += demangle (frameStrings[i])) += "\n";

    result += "\n";
    ::free (frameStrings);

    return result;
}

//==============================================================================
template<typename mutex_type>
struct checked_lock
{
    checked_lock (mutex_type& m)
        : lock (m)
    {
        if (is_real_time_context())
        {
            non_realtime_context nrtc;
            std::cerr << "!!! WARNING: Locking in real-time context\n";
            std::cerr << get_stacktrace();
        }
    }

    ~checked_lock()
    {
        if (is_real_time_context())
        {
            non_realtime_context nrtc;
            std::cerr << "!!! WARNING: Unlocking in real-time context\n";
            std::cerr << get_stacktrace();
        }
    }

private:
    std::unique_lock<mutex_type> lock;
};



//==============================================================================
// details
//==============================================================================
namespace rt_check
{
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


/* N.B. I've no idea why but this has to be present for the interposing to work... There must be something I'm missing. */
void lib_rt_check_do_malloc_free()
{
    auto m = malloc (1024);
    free (m);
}