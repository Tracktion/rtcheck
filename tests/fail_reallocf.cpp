#include <memory>
#include <rtcheck.h>

int main()
{
   #if __APPLE__
    auto res = malloc (1024);

    rtc::realtime_context rc;
    res = reallocf (res, 1024 * 4);

    return 0;
   #else
    return 1;
   #endif
}
