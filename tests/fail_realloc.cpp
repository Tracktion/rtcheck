#include <rtcheck.h>
#include <memory>

int main()
{
    volatile auto res = malloc (1024);

    rtc::realtime_context rc;
    res = realloc (res, 1024 * 4);

    return 0;
}
