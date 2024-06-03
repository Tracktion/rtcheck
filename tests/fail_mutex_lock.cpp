#include <mutex>
#include <rtcheck.h>


int main()
{
    std::mutex m;

    rtc::realtime_context rc;
    m.lock();

    return 0;
}
