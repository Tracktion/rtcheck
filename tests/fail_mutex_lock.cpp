#include <mutex>
#include <lib_rt_check.h>


int main()
{
    std::mutex m;

    rtc::realtime_context rc;
    m.lock();

    return 0;
}
