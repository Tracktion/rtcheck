#include <dlfcn.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>

#include "lib_rt_check.h"

// namespace rt_check
// {
//     std::atomic<error_mode>& get_error_mode_flag()
//     {
//         static std::atomic<error_mode> em { error_mode::exit };
//         return em;
//     }
//
//     void set_error_mode (error_mode em)
//     {
//         get_error_mode_flag().store (em, std::memory_order_release);
//     }
//
//     error_mode get_error_mode()
//     {
//         return get_error_mode_flag().load (std::memory_order_acquire);
//     }
// }

// Pointer to the real pthread_mutex_lock function
// static int (*real_pthread_mutex_lock)(pthread_mutex_t*);

// // Interposed function
// extern "C" int pthread_mutex_lock(pthread_mutex_t *mutex) 
// {
//     printf("Locking mutex at %p\n", mutex);
//     int result = real_pthread_mutex_lock(mutex);
//     printf("Mutex at %p locked\n", mutex);
//     return result;
// }

// // Load the real pthread_mutex_lock function
// __attribute__((constructor))
// void load_real_pthread_mutex_lock()
// {
    // real_pthread_mutex_lock = (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_lock");
// }

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
extern "C" void *malloc(size_t size)
{
    log_function_if_realtime_context (__func__);

    static auto real_malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
    return real_malloc(size);
}

extern "C" void free(void* ptr)
{
    log_function_if_realtime_context (__func__);

    static auto real_free = (void (*)(void*))dlsym(RTLD_NEXT, "free");
    return real_free(ptr);
}

// //==============================================================================
// // threads
// //==============================================================================
extern "C" int pthread_mutex_lock(pthread_mutex_t *mutex) 
{
    log_function_if_realtime_context (__func__);

    static auto real_pthread_mutex_lock = (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_lock");
    return real_pthread_mutex_lock(mutex);
}

extern "C" int pthread_mutex_unlock(pthread_mutex_t *mutex) 
{
    log_function_if_realtime_context (__func__);

    static auto real_pthread_mutex_unlock = (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_unlock");;
    return real_pthread_mutex_unlock(mutex);
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
    printf ("Hello librt_check!\n");
}
