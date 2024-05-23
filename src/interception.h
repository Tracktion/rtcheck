#pragma once

//==============================================================================
// The following is extracted from LLVM:
//==============================================================================

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

#ifdef __linux__
    #define INTERCEPTOR_ATTRIBUTE

    #define INTERCEPTOR(ret_type, func, ...)        \
    extern "C" INTERCEPTOR_ATTRIBUTE ret_type func(__VA_ARGS__)

    #define INTERCEPT_FUNCTION(ret_type, func, ...) static auto real = (ret_type (*)(__VA_ARGS__))dlsym(RTLD_NEXT, #func);

    #define REAL(func, ...) real __VA_ARGS__
#elif __APPLE__
    using uptr      = size_t;

    struct interpose_substitution {
        const uptr replacement;
        const uptr original;
    };

    // For a function foo() create a global pair of pointers { wrap_foo, foo } in
    // the __DATA,__interpose section.
    // As a result all the calls to foo() will be routed to wrap_foo() at runtime.
    #define INTERPOSER(func_name) __attribute__((used))     \
    const interpose_substitution substitution_##func_name[] \
    __attribute__((section("__DATA, __interpose"))) = { \
    { reinterpret_cast<const uptr>(WRAP(func_name)),    \
    reinterpret_cast<const uptr>(func_name) }         \
    }

    #define WRAP(x) wrap_##x
    #define TRAMPOLINE(x) WRAP(x)
    #define INTERCEPTOR_ATTRIBUTE
    #define DECLARE_WRAPPER(ret_type, func, ...)

    // #define REAL(func, ...) WRAP(func) (__VA_ARGS__)
    #define REAL(x) x
    #define DECLARE_REAL(ret_type, func, ...) \
    extern "C" ret_type func(__VA_ARGS__);
    #define ASSIGN_REAL(x, y)

    #define INTERCEPTOR_ZZZ(suffix, ret_type, func, ...)  \
    extern "C" ret_type func(__VA_ARGS__) suffix;       \
    extern "C" ret_type WRAP(func)(__VA_ARGS__);        \
    INTERPOSER(func);                                   \
    extern "C" INTERCEPTOR_ATTRIBUTE ret_type WRAP(func)(__VA_ARGS__)

    #define INTERCEPTOR(ret_type, func, ...) \
    INTERCEPTOR_ZZZ(/*no symbol variants*/, ret_type, func, __VA_ARGS__)

    #define INTERCEPTOR_WITH_SUFFIX(ret_type, func, ...) \
    INTERCEPTOR_ZZZ(__DARWIN_ALIAS_C(func), ret_type, func, __VA_ARGS__)

    #define INTERCEPT_FUNCTION(...)
#endif
