#include <atomic>
#include <rtcheck.h>

struct doubles
{
    double d1[4];
};

int main()
{
    std::atomic<doubles> a;
    doubles d;

    rtc::realtime_context rc;
    a.store (d);

    return 0;
}
