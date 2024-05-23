#include <dlfcn.h>
#include <stdarg.h>

#if __APPLE__
 #include <libkern/OSAtomic.h>
 #include <os/lock.h>
#endif

#include "lib_rt_check.h"
#include "interception.h"

static bool has_initialised = false;

void log_function_if_realtime_context (const char* function_name)
{
    if (! has_initialised)
        return;

    if (! is_real_time_context())
         return;

    non_realtime_context nrc;
    printf("Real-time violation: intercepted call to real-time unsafe function `%s` in real-time context! Stack trace:\n%s", function_name, get_stacktrace().c_str());

    if (rt_check::get_error_mode() == rt_check::error_mode::exit)
        std::exit (1);
}


//==============================================================================
// memory
//==============================================================================
#ifndef __APPLE__
INTERCEPTOR(void*, malloc, size_t size)
{
    log_function_if_realtime_context (__func__);
    INTERCEPT_FUNCTION(void*, malloc, size_t);

    return REAL(malloc)(size);
}
#endif


INTERCEPTOR(void*, calloc, size_t size, size_t item_size)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(void*, calloc, size_t, size_t);
    return REAL(calloc)(size, item_size);
}

INTERCEPTOR(void*, realloc, void *ptr, size_t new_size)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(void*, realloc, void*, size_t);
    return REAL(realloc)(ptr, new_size);
}

#if __APPLE__
INTERCEPTOR(void *, reallocf, void *ptr, size_t size)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(void*, reallocf, void*, size_t);
    return REAL(reallocf)(ptr, size);
}
#endif

INTERCEPTOR(void*, valloc, size_t size)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(void*, valloc, size_t);
    return REAL(valloc)(size);
}

INTERCEPTOR(void, free, void* ptr)
{
    log_function_if_realtime_context (__func__);
    INTERCEPT_FUNCTION(void, free, void*);

    return REAL(free)(ptr);
}

INTERCEPTOR(int, posix_memalign, void **memptr, size_t alignment, size_t size)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, posix_memalign, void**, size_t, size_t);
    return REAL(posix_memalign)(memptr, alignment, size);
}

INTERCEPTOR(void *, mmap, void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(void*, mmap, void*, size_t, int, int, int, off_t);
    return REAL(mmap)(addr, length, prot, flags, fd, offset);
}

INTERCEPTOR(int, munmap, void* addr, size_t length)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, munmap, void*, size_t);
    return REAL(munmap)(addr, length);
}


//==============================================================================
// threads
//==============================================================================
INTERCEPTOR(int, pthread_create, pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
    log_function_if_realtime_context (__func__);
    INTERCEPT_FUNCTION(int, pthread_create, pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
    return REAL(pthread_create)(thread, attr, start_routine, arg);
}

INTERCEPTOR(int, pthread_mutex_lock, pthread_mutex_t *mutex)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_mutex_lock, pthread_mutex_t*);
    return REAL(pthread_mutex_lock)(mutex);
}

INTERCEPTOR(int, pthread_mutex_unlock, pthread_mutex_t *mutex)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_mutex_unlock, pthread_mutex_t*);
    return REAL(pthread_mutex_unlock)(mutex);
}

INTERCEPTOR(int, pthread_join, pthread_t thread, void **value_ptr)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_join, pthread_t, void **);
    return REAL(pthread_join)(thread, value_ptr);
}

INTERCEPTOR(int, pthread_cond_signal, pthread_cond_t *cond)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_cond_signal, pthread_cond_t *);
    return REAL(pthread_cond_signal)(cond);
}

INTERCEPTOR(int, pthread_cond_broadcast, pthread_cond_t *cond)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_cond_broadcast, pthread_cond_t *);
    return REAL(pthread_cond_broadcast)(cond);
}

INTERCEPTOR(int, pthread_cond_wait, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_cond_wait, pthread_cond_t *, pthread_mutex_t *);
    return REAL(pthread_cond_wait)(cond, mutex);
}

INTERCEPTOR(int, pthread_rwlock_init, pthread_rwlock_t *rwlock,
            const pthread_rwlockattr_t *attr)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_init, pthread_rwlock_t *, const pthread_rwlockattr_t *);
    return REAL(pthread_rwlock_init)(rwlock, attr);
}

INTERCEPTOR(int, pthread_rwlock_destroy, pthread_rwlock_t *rwlock)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_destroy, pthread_rwlock_t *);
    return REAL(pthread_rwlock_destroy)(rwlock);
}

INTERCEPTOR(int, pthread_cond_timedwait, pthread_cond_t *cond,
            pthread_mutex_t *mutex, const timespec *ts)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_cond_timedwait, pthread_cond_t *,
                        pthread_mutex_t *, const timespec *);
    return REAL(pthread_cond_timedwait)(cond, mutex, ts);
}

INTERCEPTOR(int, pthread_rwlock_rdlock, pthread_rwlock_t *lock)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_rdlock, pthread_rwlock_t *);
    return REAL(pthread_rwlock_rdlock)(lock);
}

INTERCEPTOR(int, pthread_rwlock_unlock, pthread_rwlock_t *lock)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_unlock, pthread_rwlock_t *);
    return REAL(pthread_rwlock_unlock)(lock);
}

INTERCEPTOR(int, pthread_rwlock_wrlock, pthread_rwlock_t *lock)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, pthread_rwlock_wrlock, pthread_rwlock_t *);
    return REAL(pthread_rwlock_wrlock)(lock);
}


#ifndef __APPLE__
INTERCEPTOR(int, pthread_spin_lock, pthread_spinlock_t *spinlock)
{
    log_function_if_realtime_context (__func__);
    INTERCEPT_FUNCTION(int, pthread_spin_lock, pthread_spinlock_t*);
    return REAL(pthread_spin_lock)(spinlock);
}

INTERCEPTOR(int, futex, int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, futex, int*, int, int, const struct timespec*, int*, int);
    return REAL(futex)(uaddr, op, val, timeout, uaddr2, val3);
}
#endif

//==============================================================================
// sleep
//==============================================================================
INTERCEPTOR(unsigned int, sleep, unsigned int seconds)
{
    log_function_if_realtime_context (__func__);

    static auto real = (unsigned int (*)(unsigned int))dlsym(RTLD_NEXT, "sleep");
    return REAL(sleep)(seconds);
}

INTERCEPTOR(int, usleep, useconds_t useconds)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, usleep, useconds_t);
    return REAL(usleep)(useconds);
}

INTERCEPTOR(int, nanosleep, const struct timespec *req, struct timespec * rem)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, nanosleep, const struct timespec *, struct timespec *);
    return REAL(nanosleep)(req, rem);
}

//==============================================================================
// files
//==============================================================================
INTERCEPTOR(int, stat, const char* pathname, struct stat* statbuf)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, stat, const char*, struct stat*);
    return REAL(stat)(pathname, statbuf);
}

INTERCEPTOR(int, fstat, int fd, struct stat *statbuf)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, fstat, int, struct stat*);
    return REAL(fstat)(fd, statbuf);
}

INTERCEPTOR(int, open, const char *path, int oflag, ...)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(int, open, const char*, int, ...);

    va_list args;
    va_start(args, oflag);
    auto result = REAL(open)(path, oflag, args);
    va_end(args);

    return result;
}

INTERCEPTOR(FILE*, fopen, const char *path, const char *mode)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(FILE*, fopen, const char*, const char*);
    auto result = REAL(fopen)(path, mode);

    return result;
}

INTERCEPTOR(int, openat, int fd, const char *path, int oflag, ...)
{
    log_function_if_realtime_context (__func__);

    va_list args;
    va_start(args, oflag);
    INTERCEPT_FUNCTION(int, openat, int, const char*, int, ...);
    auto result = REAL(openat)(fd, path, oflag, args);
    va_end(args);

    return result;
}


//==============================================================================
// system
//==============================================================================
#ifndef __APPLE__
INTERCEPTOR(long, schedule, void)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(long, schedule, void);
    return REAL(schedule)();
}

INTERCEPTOR(long, context_switch, struct task_struct *prev, struct task_struct *next)
{
    log_function_if_realtime_context (__func__);

    INTERCEPT_FUNCTION(long, context_switch, struct task_struct *, struct task_struct *);
    return REAL(context_switch)(prev, next);
}
#endif

INTERCEPTOR(long int, syscall, long int sid, ...)
{
    log_function_if_realtime_context (__func__);

    va_list args;
    va_start(args, sid);
    INTERCEPT_FUNCTION(long, syscall, long, ...);
    va_end(args);

    return REAL(syscall)(sid);
}


//==============================================================================
// Apple
//==============================================================================
#if __APPLE__

#pragma clang diagnostic push
// OSSpinLockLock is deprecated, but still in use in libc++
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

INTERCEPTOR(void, OSSpinLockLock, volatile OSSpinLock *lock)
{
    log_function_if_realtime_context (__func__);
    return REAL(OSSpinLockLock)(lock);
}

INTERCEPTOR(void, os_unfair_lock_lock, os_unfair_lock_t lock)
{
    log_function_if_realtime_context (__func__);
    return REAL(os_unfair_lock_lock)(lock);
}

#pragma clang diagnostic pop

#endif

//==============================================================================
// init
//==============================================================================
__attribute__((constructor))
void init()
{
    printf ("Hello librt_check!\n");
    has_initialised = true;
}

