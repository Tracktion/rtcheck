#include <functional>
#include <lib_rt_check.h>

struct doubles
{
    double d[8];
};

int main()
{
    rtc::realtime_context rc;
    doubles d;
    std::function<void()> fn = [d] { };

    return 0;
}
