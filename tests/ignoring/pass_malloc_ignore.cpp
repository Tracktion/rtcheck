#include <memory>
#include <lib_rt_check.h>

int main()
{
    rtc::realtime_context rc;
    rtc::disable_checks_for_thread (rtc::check_flags::malloc);

    [[ maybe_unused ]] auto res = malloc (1024);

    return 0;
}
