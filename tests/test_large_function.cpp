#include <functional>
#include <lib_rt_check.h>

struct doubles
{
    double d[4];
};

int main()
{
    std::function<void()> fn;

    {
        realtime_context rc;
        doubles d;
        std::function<void()> fn = [d] { };

    }

    return 0;
}
