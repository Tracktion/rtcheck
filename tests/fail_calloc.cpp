#include <lib_rt_check.h>

int main()
{
    realtime_context rc;

    [[ maybe_unused ]] auto res = calloc (1024, 4);

    return 0;
}
