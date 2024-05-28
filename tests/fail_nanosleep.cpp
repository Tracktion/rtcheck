#include <unistd.h>
#include <time.h>
#include <lib_rt_check.h>

int main()
{
    timespec req;

    rtc::realtime_context rc;
    nanosleep(&req, nullptr);

    return 0;
}
