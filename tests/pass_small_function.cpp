#include <functional>
#include <rtcheck.h>

struct doubles
{
    double d[2];
};

int main()
{
    rtc::realtime_context rc;
    doubles d;
    std::function<void()> fn = [d] { };

    return 0;
}
