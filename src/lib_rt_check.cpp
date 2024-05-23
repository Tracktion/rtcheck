#include <dlfcn.h>
#include <stdarg.h>

#include "lib_rt_check.h"
#include "interception.h"


inline void log_function_if_realtime_context (const char* function_name)
{
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
// extern "C" void *malloc(size_t size)
// {
//     log_function_if_realtime_context (__func__);
//
//     static auto real = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
//     return real(size);
// }


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

#ifndef __APPLE__
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
extern "C" unsigned int sleep(unsigned int seconds)
{
    log_function_if_realtime_context (__func__);

    static auto real = (unsigned int (*)(unsigned int))dlsym(RTLD_NEXT, "sleep");
    return real(seconds);
}

extern "C" int usleep(useconds_t useconds)
{
    log_function_if_realtime_context (__func__);

    static auto real = (int (*)(useconds_t))dlsym(RTLD_NEXT, "usleep");
    return real(useconds);
}

extern "C" int nanosleep(const struct timespec *req,
                         struct timespec * rem)
{
    log_function_if_realtime_context (__func__);

    static auto real = (int (*)(const struct timespec *, struct timespec *))dlsym(RTLD_NEXT, "nanosleep");
    return real(req, rem);
}

//==============================================================================
// files
//==============================================================================
extern "C" int stat(const char* pathname, struct stat* statbuf)
{
    log_function_if_realtime_context (__func__);

    static auto real_stat = (int (*)(const char*, struct stat*))dlsym(RTLD_NEXT, "stat");
    return real_stat (pathname, statbuf);
}

extern "C" int fstat(int fd, struct stat *statbuf)
{
    log_function_if_realtime_context (__func__);

    static auto real_fstat = (int (*)(int, struct stat*))dlsym(RTLD_NEXT, "fstat");
    return real_fstat (fd, statbuf);
}

extern "C" int open(const char *path, int oflag, ...)
{
    log_function_if_realtime_context (__func__);

    va_list args;
    va_start(args, oflag);
    static auto real_open = (int (*)(const char*, int, ...))dlsym(RTLD_NEXT, "open");
    auto result = real_open (path, oflag, args);
    va_end(args);

    return result;
}

extern "C" FILE* fopen(const char *path, const char *mode)
{
    log_function_if_realtime_context (__func__);

    static auto real_fopen = (FILE* (*)(const char*, const char*))dlsym(RTLD_NEXT, "fopen");
    auto result = real_fopen (path, mode);

    return result;
}

extern "C" int openat(int fd, const char *path, int oflag, ...)
{
    log_function_if_realtime_context (__func__);

    va_list args;
    va_start(args, oflag);
    static auto real_openat = (int (*)(int, const char*, int, ...))dlsym(RTLD_NEXT, "openat");
    auto result = real_openat (fd, path, oflag, args);
    va_end(args);

    return result;
}


//==============================================================================
// system
//==============================================================================
extern "C" long schedule(void)
{
    log_function_if_realtime_context (__func__);

    static auto real = (long (*)(void))dlsym(RTLD_NEXT, "schedule");
    return real();
}

extern "C" long context_switch(struct task_struct *prev, struct task_struct *next)
{
    log_function_if_realtime_context (__func__);

    static auto real = (long (*)(struct task_struct *, struct task_struct *))dlsym(RTLD_NEXT, "context_switch");
    return real(prev, next);
}

extern "C" long int syscall(long int sid, ...)
{
    log_function_if_realtime_context (__func__);

    va_list args;
    va_start(args, sid);
    static auto real_syscall = (long (*)(long, ...))dlsym(RTLD_NEXT, "syscall");
    va_end(args);

    return real_syscall (sid);
}

//==============================================================================
// init
//==============================================================================
__attribute__((constructor))
void init()
{
    // INTERCEPT_FUNCTION(malloc);
    // INTERCEPT_FUNCTION(free)
    // INTERCEPT_FUNCTION(stat)
    printf ("Hello librt_check!\n");
}

