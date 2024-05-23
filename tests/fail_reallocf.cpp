#include <lib_rt_check.h>

int main()
{
    auto res = malloc (1024);

    realtime_context rc;
    res = reallocf (res, 1024 * 4);

    return 0;
}
