# lib_rt_checked
Dynamic library to catch run-time safety violations heavily inspired by [RADSan](https://github.com/realtime-sanitizer/radsan)

## Features
- [ ] Enable in scope
- [ ] Disable in scope
- [ ] Symbolicated stack-trace
- [ ] Build-time flags for checks
- [ ] Run-time options for checks
- [ ] Opt-in for own code
- [ ] linux
- [ ] macOS

## Functions
- Time
  - [x] sleep
  - [x] nanosleep
  - [x] usleep
- Memory
  - [x] malloc
  - [x] calloc
  - [x] realloc
  - [x] free
  - [ ] reallocf (macOS)
  - [x] valloc
  - [x] posix_memalign
  - [x] mmap
  - [x] munmap
- Threads
  - [ ] pthread_create
  - [ ] pthread_mutex_lock
  - [ ] pthread_mutex_unlock
  - [ ] pthread_join
  - [ ] pthread_cond_signal
  - [ ] pthread_cond_broadcast
  - [ ] pthread_cond_wait
  - [ ] pthread_cond_timedwait
  - [ ] pthread_rwlock_rdlock
  - [ ] pthread_rwlock_unlock
  - [ ] pthread_rwlock_wrlock
  - [ ] pthread_spin_lock
- Files
  - [ ] open
  - [ ] openat
  - [ ] close
  - [ ] fopen
  - [ ] fread
  - [ ] fwrite
  - [ ] fclose
  - [ ] fcntl
  - [ ] creat
  - [ ] puts
  - [ ] fputs
  - [x] stat
  - [ ] stat64
  - [ ] fstat
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
  - [x] syscall

## CI/Tests
- [ ] Failures
  - [x] Throwing exceptions
  - [x] Large std::function
  - [x] Atomic 4*ptr size
  - [ ] Dynamic loading of a library
- [ ] Passes
  - [x] Atomic double
  - [x] Small std::function
- [x] Running on CI Linux
- [x] Running on CI macOS
- [x] Tests via CTest (can catch failues)

## Packages
- [ ] cpack
- [ ] conan
