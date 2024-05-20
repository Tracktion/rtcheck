#include <lib_rt_check.h>


int main()
{
    auto res = malloc (1024);

    realtime_context rc;
    free (res);

    return 0;
}
