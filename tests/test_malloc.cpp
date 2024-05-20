#include <lib_rt_check.h>

int main()
{
    realtime_context rc;

    [[ maybe_unused ]] auto res = malloc (1024);

    return 0;
}
