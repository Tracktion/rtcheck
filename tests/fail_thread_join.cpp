#include <thread>
#include <lib_rt_check.h>

int main()
{
    std::thread t ([] {});

    rtc::realtime_context rc;
    t.join();

    return 0;
}