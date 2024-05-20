#include <atomic>
#include <lib_rt_check.h>

struct doubles
{
    double d1, d2, d3, d4;
};

int main()
{
    std::atomic<doubles> a;
    doubles d;

    realtime_context rc;
    a.store (d);

    return 0;
}
