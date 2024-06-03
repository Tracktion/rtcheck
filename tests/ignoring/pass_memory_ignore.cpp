#include <memory>
#include <rtcheck.h>

int main()
{
    rtc::realtime_context rc;
    rtc::disable_checks_for_thread (rtc::check_flags::memory);

    auto res = malloc (1024);
    res = realloc(res, 1024 * 2);
    free(res);

    return 0;
}
