#include <mutex>
#include <rtcheck.h>


int main()
{
    std::mutex m;
    m.lock();

    rtc::realtime_context rc;
    m.unlock();

    return 0;
}
