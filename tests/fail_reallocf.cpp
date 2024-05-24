#include <lib_rt_check.h>

int main()
{
   #if __APPLE_
    auto res = malloc (1024);

    realtime_context rc;
    res = reallocf (res, 1024 * 4);

    return 0;
   #else
    return 1;
   #endif
}
