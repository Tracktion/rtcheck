[![Build](https://github.com/Tracktion/rtcheck/actions/workflows/build.yml/badge.svg)](https://github.com/Tracktion/rtcheck/actions/workflows/build.yml)

# rtcheck
Dynamic library to catch run-time safety violations heavily inspired by [RADSan](https://github.com/realtime-sanitizer/radsan)

## Contents
- [Adding rtcheck to a project](#adding-rtcheck-to-a-project)
- [Using rtcheck](#using-rtcheck)
- [Disabling checks](#disabling-checks)
- [Catching your own violations](#catching-your-own-violations)
- [Error modes](#error-modes)

## Adding rtcheck to a project
### CMake option 1: Git Submodule
```
git submodule add -b main https://github.com/Tracktion/rtcheck rtcheck
```

To update down the road:
```
git submodule update --remote --merge rtcheck
```

# Build rtcheck as part of your CMakeLists.txt
```cmake
add_subdirectory(rtcheck)
```

### CMake option 2: FetchContent
```cmake
Include(FetchContent)
FetchContent_Declare(rtcheck
    GIT_REPOSITORY https://github.com/Tracktion/rtcheck.git
    GIT_TAG origin/main
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/rtcheck)
FetchContent_MakeAvailable(rtcheck)
```

### Link rtcheck to your CMake project
```cmake
target_link_libraries("YourProject" PRIVATE rtcheck)
```

Then include rtcheck in your source files:
```c++
#include <rtcheck.h>
```

## Using rtcheck
Simply add an instance of `rtc::realtime_context` at the start of the scope you want to check for real-time safety violations like this:  
```c++
int main()
{
    std::thread t ([]
    {
        rtc::realtime_context rc;
        malloc (1024); // Allocating memory is a real-time safety violation!
    });

    t.join();

    return 0;
}
```
The call to malloc in the above code will then be caught and logged as so:
```
Real-time violation: intercepted call to real-time unsafe function malloc in real-time context! Stack trace:
0   librt_check.dylib                   0x000000010543f7d4 _ZN3rtc14get_stacktraceEv + 84
 1   librt_check.dylib                   0x000000010543f450 _ZN3rtc32log_function_if_realtime_contextEPKc + 300
  2   librt_check.dylib                   0x000000010543fe00 _Z44log_function_if_realtime_context_and_enabledN3rtc11check_flagsEPKc + 64
   3   librt_check.dylib                   0x000000010543fe30 wrap_malloc + 32
    4   fail_malloc                         0x0000000104fcff14 main + 32
     5   dyld                                0x00000001901760e0 start + 2360
```

## Disabling checks
There are two ways to disable checks. You can either disable all checks, which can be useful if you 
know you'll be calling a potentially unsafe function but in a safe way (e.g. an un-contented lock), or you want to log something:
```c++
{
    rtc::non_realtime_context nrc;
    std::cout << "need to get this message on the screen!";
}
```
Or you can selectively disable checks:
```c++
{
    rtc::disable_checks_for_thread (check_flags::threads);
    mutex.lock(); // I know this is uncontended, don't for get to unlock!
}
```

## Catching your own violations
If you have some code which you know is non-real-time safe e.g. an unbounded distribution function or some other async call, you can opt-in to let rtcheck catch it by calling the following function:
```c++
void my_unsafe_function()
{
    log_function_if_realtime_context (__func__);
}
```
This will then get logged if called whilst a `rtc::realtime_context` is alive.

## Error Modes
There are two currently supported error modes
- Exit with error code 1 (default)
- Log and continue

Exiting it useful for CI runs where you want the program to terminate in an obvious way (non-zero exit code) to fail the run.
In normal use though, you may just want to log the violation and continue.

You can change between these globally using the following function:
```c++
/** Sets the global error more to determine the behaviour when a real-time
    violation is detected.
*/
void set_error_mode (error_mode);
```
---
# Notes:
## Features
- [x] Enable in scope
- [x] Disable in scope
- [x] Symbolicated stack-trace
- [x] Run-time options for checks
- [x] Opt-in for own code
- [x] linux
- [x] macOS (malloc unsupported)
- [ ] Delay time

## Functions (test = ✔)
- Time
  - [x] sleep ✔
  - [x] nanosleep ✔
  - [x] usleep ✔
- Memory
  - [x] malloc ✔
  - [x] calloc ✔
  - [x] realloc ✔
  - [x] free ✔
  - [x] reallocf (macOS) ✔
  - [x] valloc ✔
  - [x] posix_memalign ✔
  - [x] mmap ✔
  - [x] munmap ✔
- Threads
  - [x] pthread_create ✔
  - [x] pthread_mutex_lock ✔
  - [x] pthread_mutex_unlock ✔
  - [x] pthread_join ✔
  - [x] pthread_cond_signal
  - [x] pthread_cond_broadcast
  - [x] pthread_cond_wait
  - [x] pthread_cond_timedwait
  - [x] pthread_rwlock_rdlock ✔
  - [x] pthread_rwlock_unlock ✔
  - [x] pthread_rwlock_wrlock ✔
  - [x] pthread_spin_lock (linux)
- Files
  - [x] open ✔
  - [x] openat
  - [ ] close
  - [x] fopen
  - [ ] fread
  - [ ] fwrite
  - [ ] fclose
  - [x] fcntl ✔
  - [ ] creat
  - [ ] puts
  - [ ] fputs
  - [x] stat ✔
  - [ ] stat64
  - [x] fstat
  - [ ] fstat64
- IO
  - [ ] socket
  - [ ] send
  - [ ] sendmsg
  - [ ] sendto
  - [ ] recv
  - [ ] recvmsg
  - [ ] recvfrom
  - [ ] shutdown
- System calls 
  - [x] syscall ✔
  - [x] schedule
  - [x] context_switch

## CI/Tests
- Failures
  - [x] Throwing exceptions
  - [x] Large std::function
  - [x] Atomic 4*ptr size
  - [ ] Dynamic loading of a library
- Passes
  - [x] Atomic double
  - [x] Small std::function
  - [x] thread_local? (pass on linux, fail on macOS)
- [x] Running on CI Linux
- [x] Running on CI macOS
- [x] Tests via CTest (can catch failues)

## Packages
- [ ] cpack
- [ ] conan
