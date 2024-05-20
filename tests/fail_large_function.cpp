#include <functional>
#include <lib_rt_check.h>

struct doubles
{
    double d[4];
};

int main()
{
    realtime_context rc;
    doubles d;
    std::function<void()> fn = [d] { };

    return 0;
}
