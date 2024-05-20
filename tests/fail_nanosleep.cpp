#include <unistd.h>
#include <lib_rt_check.h>

int main()
{
    timespec req;

    realtime_context rc;
    nanosleep(&req, nullptr);

    return 0;
}
