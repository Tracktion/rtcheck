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
- [ ] syscall (futex)

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
