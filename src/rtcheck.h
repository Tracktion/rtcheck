#pragma once

#include <cstdint>

namespace rtc
{
    //==============================================================================
    //==============================================================================
    /** Puts the current thread in to a real-time context.
        You'd generally put one of these at the start of your real-time thread which
        will enable checking of real-time safety violations.
     */
    struct realtime_context
    {
        /** Enters the real-time context. */
        realtime_context();

        /** Exits the real-time context. */
        ~realtime_context();
    };

    //==============================================================================
    /** Exits a real-time context for the life-time of the object.
        Placing one of these in the scope of a known real-time unsafe call can avoid
        flagging errors. This can enable you to progressively adopt real-time safety
        or to perform logging etc. in debug builds.
     */
    struct non_realtime_context
    {
        /** Exits the real-time context.
            pre_condition: Thread is in a real-time context
        */
        non_realtime_context();

        /** Re-enters the real-time context. */
        ~non_realtime_context();
    };

    //==============================================================================
    //==============================================================================
    /** Returns true if the current thread is in a real-time context. */
    bool is_real_time_context();

    /** If the current thead is in a real-time context, this will log and carry out
        the currently error_mode behaviour.
        This can be used in your own code to flag functions that are not real-time safe.
     */
    void log_function_if_realtime_context (const char* function_name);

    //==============================================================================
    //==============================================================================
    /** Holds the various supported error modes. */
    enum class error_mode
    {
        exit,   /// Exit with value 1, default
        cont    /// Continue execution
    };

    /** Sets the global error more to determine the behaviour when a real-time
        violation is detected.
    */
    void set_error_mode (error_mode);

    /** Returns the global error detection mode. */
    error_mode get_error_mode();


    //==============================================================================
    //==============================================================================
    /** Enum specifying the different checks which can be used to individually or as
       a group be disabled for the current thread.
     */
    enum class check_flags : uint64_t
    {
        //==============================================================================
        // memory
        //==============================================================================
        malloc                          = 1 << 0,
        calloc                          = 1 << 1,
        realloc                         = 1 << 2,
        reallocf                        = 1 << 3, // apple only
        valloc                          = 1 << 4,
        free                            = 1 << 5,
        posix_memalign                  = 1 << 6,
        mmap                            = 1 << 7,
        munmap                          = 1 << 8,

        memory                          = malloc | calloc | realloc | reallocf | valloc
                                            | free | posix_memalign | mmap | munmap,

        //==============================================================================
        // threads
        //==============================================================================
        pthread_create                  = 1 << 9,
        pthread_mutex_lock              = 1 << 10,
        pthread_mutex_unlock            = 1 << 11,
        pthread_join                    = 1 << 12,
        pthread_cond_signal             = 1 << 13,
        pthread_cond_broadcast          = 1 << 14,
        pthread_cond_wait               = 1 << 15,
        pthread_rwlock_init             = 1 << 16,
        pthread_rwlock_destroy          = 1 << 17,
        pthread_cond_timedwait          = 1 << 18,
        pthread_rwlock_rdlock           = 1 << 19,
        pthread_rwlock_unlock           = 1 << 20,
        pthread_rwlock_wrlock           = 1 << 21,
        pthread_spin_lock               = 1 << 22,      // linux only
        futex                           = 1 << 23,      // linux only
        OSSpinLockLock                  = 1ull << 35,   // apple only
        os_unfair_lock_lock             = 1ull << 36,   // apple only

        threads = pthread_create | pthread_mutex_lock | pthread_mutex_unlock | pthread_join
                    | pthread_cond_signal | pthread_cond_broadcast | pthread_cond_wait
                    | pthread_rwlock_init | pthread_rwlock_destroy | pthread_cond_timedwait
                    | pthread_rwlock_rdlock | pthread_rwlock_unlock | pthread_rwlock_wrlock
                    | pthread_spin_lock | futex | OSSpinLockLock | os_unfair_lock_lock,

        //==============================================================================
        // sleeping
        //==============================================================================
        sleep                           = 1 << 24,
        usleep                          = 1 << 25,
        nanosleep                       = 1 << 26,

        sleeping                        = sleep | usleep | nanosleep,

        //==============================================================================
        // files
        //==============================================================================
        stat                            = 1 << 27,
        fstat                           = 1 << 28,
        open                            = 1 << 29,
        fopen                           = 1 << 30,
        openat                          = 1ull << 31,

        files                           = stat | fstat | open | fopen | openat,

        //==============================================================================
        // system
        //==============================================================================
        schedule                        = 1ull << 32,  // linux only
        context_switch                  = 1ull << 33,  // linux only
        syscall                         = 1ull << 34,

        sys                             = schedule | context_switch | syscall
    };

    /** Disables a number of checks for the current thread. */
    void disable_checks_for_thread (uint64_t flags);

    /** Disables a check for the current thread. */
    void disable_checks_for_thread (check_flags);

    /** Returns true if the current check is enabled. */
    [[nodiscard]] bool is_check_enabled_for_thread (check_flags);
}
