#include <lib_rt_check.h>
#include <memory>

int main()
{
    rtc::realtime_context rc;

    [[ maybe_unused ]] volatile auto res = calloc (1024, 4);

    return 0;
}
