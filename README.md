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
  - [ ] Throwing exceptions
  - [ ] Large std::function
  - [ ] Atomic 4*ptr size
- [ ] Passes
  - [ ] Atomic double
  - [ ] Small std::function
- [ ] Running on CI
- [ ] Tests via CTest (can catch failues)

## Packages
- [ ] cpack
- [ ] conan
