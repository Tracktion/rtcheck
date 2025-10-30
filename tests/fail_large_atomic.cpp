#include <atomic>
#include <cassert>
#include <array>
#include <rtcheck.h>

struct doubles
{
    double d1[4];
};

int main()
{
    std::atomic<doubles> a;
    assert (! a.is_lock_free());

    doubles d { 1.0, 2.0, 3.0, 4.0 };

    rtc::realtime_context rc;
    a.store (d);

    return 0;
}
