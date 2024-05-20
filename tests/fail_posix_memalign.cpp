#include <lib_rt_check.h>

int main()
{
    realtime_context rc;

    void* p;
    [[ maybe_unused ]] auto res = posix_memalign (&p, 32, 128);

    return 0;
}
