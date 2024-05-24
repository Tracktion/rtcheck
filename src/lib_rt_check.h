#pragma once

#include <memory>
#include <atomic>
#include <cassert>
#include <string>
#include <iostream>
#include <mutex>
#include <execinfo.h>

#if __has_include (<cxxabi.h>)
 #include <cxxabi.h>
#endif

namespace rt_check
{
    enum class error_mode
    {
        exit,   /// Exit with value 1, default
        cont    /// Continue execution
    };

    void set_error_mode (error_mode);
    error_mode get_error_mode();
}

//==============================================================================
/**
    Used to enable intercepting of allocations using new, new[], delete and
    delete[] on the calling thread.
*/
struct realtime_context_state
{
    realtime_context_state() = default;

    /// Enters a real-time context
    void realtime_enter()               { realtime_flag.store (true); }

    /// Exits a real-time context
    void realtime_exit()                { realtime_flag.store (false); }

    /// Returns true if this is in a real-time state
    bool is_realtime_context() const    { return realtime_flag.load(); }

    //==============================================================================
private:
    std::atomic<bool> realtime_flag { false };
};

//==============================================================================
/** Returns a realtime_context_state for the current thread. */
realtime_context_state& get_realtime_context_state();

//==============================================================================
inline bool is_real_time_context()
{
    return get_realtime_context_state().is_realtime_context();
}

//==============================================================================
struct realtime_context
{
    realtime_context()
    {
        get_realtime_context_state().realtime_enter();
    }

    ~realtime_context()
    {
        get_realtime_context_state().realtime_exit();
    }
};

//==============================================================================
struct non_realtime_context
{
    non_realtime_context()
    {
        assert (get_realtime_context_state().is_realtime_context());
        get_realtime_context_state().realtime_exit();
    }

    ~non_realtime_context()
    {
        get_realtime_context_state().realtime_enter();
    }
};


//==============================================================================
//==============================================================================
void log_function_if_realtime_context (const char* function_name);


//==============================================================================
inline std::string demangle (std::string name)
{
   #if __has_include (<cxxabi.h>)
    int status;

    if (char* demangled = abi::__cxa_demangle (name.c_str(), nullptr, nullptr, &status); status == 0)
    {
        std::string demangledString (demangled);
        free (demangled);
        return demangledString;
    }
   #endif

    return name;
}

inline std::string get_stacktrace()
{
    std::string result;

    void* stack[128];
    auto frames = backtrace (stack, 128);
    char** frameStrings = backtrace_symbols (stack, frames);

    for (auto i = (decltype (frames)) 0; i < frames; ++i)
        (result.append (i, ' ') += demangle (frameStrings[i])) += "\n";

    result += "\n";
    ::free (frameStrings);

    return result;
}

//==============================================================================
template<typename mutex_type>
struct checked_lock
{
    checked_lock (mutex_type& m)
        : lock (m)
    {
        if (is_real_time_context())
        {
            non_realtime_context nrtc;
            std::cerr << "!!! WARNING: Locking in real-time context\n";
            std::cerr << get_stacktrace();
        }
    }

    ~checked_lock()
    {
        if (is_real_time_context())
        {
            non_realtime_context nrtc;
            std::cerr << "!!! WARNING: Unlocking in real-time context\n";
            std::cerr << get_stacktrace();
        }
    }

private:
    std::unique_lock<mutex_type> lock;
};



//==============================================================================
namespace rt_check
{
std::atomic<error_mode>& get_error_mode_flag()
{
    static std::atomic<error_mode> em { error_mode::exit };
    return em;
}

void set_error_mode (error_mode em)
{
    get_error_mode_flag().store (em, std::memory_order_release);
}

error_mode get_error_mode()
{
    return get_error_mode_flag().load (std::memory_order_acquire);
}
}


/* N.B. I've no idea why but this has to be present for the interposing to work... There must be something I'm missing. */
void lib_rt_check_do_malloc_free()
{
    auto m = malloc (1024);
    free (m);
}