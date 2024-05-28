# lib_rt_checked
Dynamic library to catch run-time safety violations heavily inspired by [RADSan](https://github.com/realtime-sanitizer/radsan)

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
  - [x] open
  - [x] openat
  - [ ] close
  - [x] fopen
  - [ ] fread
  - [ ] fwrite
  - [ ] fclose
  - [ ] fcntl
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
  - [ ] thread_local? (pass on linux, fail on macOS)
- [x] Running on CI Linux
- [x] Running on CI macOS
- [x] Tests via CTest (can catch failues)

## Packages
- [ ] cpack
- [ ] conan
