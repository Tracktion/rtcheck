#include <lib_rt_check.h>
#include <memory>


int main()
{
    rtc::realtime_context rc;

    [[ maybe_unused ]] volatile auto res = malloc (1024);

    return 0;
}
