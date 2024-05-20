#include <mutex>
#include <lib_rt_check.h>


int main()
{
    std::mutex m;
    m.lock();

    realtime_context rc;
    m.unlock();

    return 0;
}
