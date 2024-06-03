#include <rtcheck.h>
#include <memory>

int main()
{
    volatile auto res = malloc (1024);

    rtc::realtime_context rc;
    free (res);

    return 0;
}
