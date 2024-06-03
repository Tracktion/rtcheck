#include <stdlib.h>
#include <rtcheck.h>

int main()
{
    rtc::realtime_context rc;

    void* p;
    [[ maybe_unused ]] auto res = posix_memalign (&p, 32, 128);

    return 0;
}
