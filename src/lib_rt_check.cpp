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

// The following is extracted from LLVM:
//===-- interception.h ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
//
// Machinery for providing replacements/wrappers for system functions.
//===----------------------------------------------------------------------===//

//==============================================================================
// macOS
//==============================================================================
// #ifdef __APPLE__
// #include <sys/cdefs.h>
//
// #define SIZE_T size_t;
//
// using uptr      = SIZE_T;
// // using sptr      = SSIZE_T;
// // using sptr      = PTRDIFF_T;
// // using s64       = INTMAX_T;
// // using u64       = UINTMAX_T;
// // using OFF_T     = OFF_T;
// // using OFF64_T   = OFF64_T;
//
// struct interpose_substitution {
//     const uptr replacement;
//     const uptr original;
// };
//
// // For a function foo() create a global pair of pointers { wrap_foo, foo } in
// // the __DATA,__interpose section.
// // As a result all the calls to foo() will be routed to wrap_foo() at runtime.
// #define INTERPOSER(func_name) __attribute__((used))     \
// const interpose_substitution substitution_##func_name[] \
// __attribute__((section("__DATA, __interpose"))) = { \
// { reinterpret_cast<const uptr>(WRAP(func_name)),    \
// reinterpret_cast<const uptr>(func_name) }         \
// }
//
// // For a function foo() and a wrapper function bar() create a global pair
// // of pointers { bar, foo } in the __DATA,__interpose section.
// // As a result all the calls to foo() will be routed to bar() at runtime.
// #define INTERPOSER_2(func_name, wrapper_name) __attribute__((used)) \
// const interpose_substitution substitution_##func_name[]             \
// __attribute__((section("__DATA, __interpose"))) = {             \
// { reinterpret_cast<const uptr>(wrapper_name),                   \
// reinterpret_cast<const uptr>(func_name) }                     \
// }
//
// # define WRAP(x) wrap_##x
// # define TRAMPOLINE(x) WRAP(x)
// # define INTERCEPTOR_ATTRIBUTE
// # define DECLARE_WRAPPER(ret_type, func, ...)
//
// # define REAL(x) x
// # define DECLARE_REAL(ret_type, func, ...) \
// extern "C" ret_type func(__VA_ARGS__);
// # define ASSIGN_REAL(x, y)
//
// #define INTERCEPTOR_ZZZ(suffix, ret_type, func, ...)  \
// extern "C" ret_type func(__VA_ARGS__) suffix;       \
// extern "C" ret_type WRAP(func)(__VA_ARGS__);        \
// INTERPOSER(func);                                   \
// extern "C" INTERCEPTOR_ATTRIBUTE ret_type WRAP(func)(__VA_ARGS__)
//
// #define INTERCEPTOR(ret_type, func, ...) \
// INTERCEPTOR_ZZZ(/*no symbol variants*/, ret_type, func, __VA_ARGS__)
//
// #define INTERCEPTOR_WITH_SUFFIX(ret_type, func, ...) \
// INTERCEPTOR_ZZZ(__DARWIN_ALIAS_C(func), ret_type, func, __VA_ARGS__)
//
// // Override |overridee| with |overrider|.
// #define OVERRIDE_FUNCTION(overridee, overrider) \
// INTERPOSER_2(overridee, WRAP(overrider))
//
// INTERCEPTOR(void, free, void *ptr)
// {
//     if (ptr != NULL)
//         log_function_if_realtime_context (__func__);
//
//     return REAL(free)(ptr);
// }
//
// INTERCEPTOR(void*, malloc, size_t size)
// {
//     log_function_if_realtime_context (__func__);
//     return REAL(malloc)(size);
// }
//
// INTERCEPTOR(int, stat, const char* pathname, struct stat* statbuf)
// {
//     log_function_if_realtime_context (__func__);
//     return REAL(stat)(pathname, statbuf);
// }
//
// #define INTERCEPT_FUNCTION_MAC(func)
// #define INTERCEPT_FUNCTION_VER_MAC(func, symver)
//
// # define INTERCEPT_FUNCTION(func) INTERCEPT_FUNCTION_MAC(func)
// # define INTERCEPT_FUNCTION_VER(func, symver) \
// INTERCEPT_FUNCTION_VER_MAC(func, symver)
//
// //==============================================================================
// // init
// //==============================================================================
// __attribute__((constructor))
// void init()
// {
//     INTERCEPT_FUNCTION(malloc)
//     INTERCEPT_FUNCTION(free)
//     INTERCEPT_FUNCTION(stat)
//     printf ("Hello librt_check!\n");
// }
//
// #endif //__APPLE__

