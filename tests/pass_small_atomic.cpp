#include <atomic>
#include <lib_rt_check.h>


int main()
{
    std::atomic<float> a;

    realtime_context rc;
    a = 42.0f;

    return 0;
}
