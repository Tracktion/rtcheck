#include <unistd.h>
#include <time.h>
#include <rtcheck.h>

int main()
{
    timespec req;

    rtc::realtime_context rc;
    nanosleep(&req, nullptr);

    return 0;
}
