#include <vector>

#include <lib_rt_check.h>


int main()
{
    std::vector<int> vec;

    {
        rtc::realtime_context rc;
        vec.reserve (42);
    }

    return 0;
}
