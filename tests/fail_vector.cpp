#include <vector>

#include <rtcheck.h>


int main()
{
    std::vector<int> vec;

    {
        rtc::realtime_context rc;
        vec.reserve (42);
    }

    return 0;
}
