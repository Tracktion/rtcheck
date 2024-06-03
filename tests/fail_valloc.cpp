#include <memory>
#include <rtcheck.h>

int main()
{
    rtc::realtime_context rc;

    [[ maybe_unused ]] volatile auto res = valloc (1024);

    return 0;
}
