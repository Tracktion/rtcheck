#include <thread>
#include <lib_rt_check.h>

int main()
{
    std::thread t;

    {
        realtime_context rc;
        t = std::thread ([] {});
    }

    t.join();

    return 0;
}
