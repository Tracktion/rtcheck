#include <cassert>
#include <atomic>
#include <rtcheck.h>


int main()
{
    std::atomic<float> a;
    assert(a.is_lock_free());

    rtc::realtime_context rc;
    a = 42.0f;

    return 0;
}
