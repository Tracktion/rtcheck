#include <chrono>
#include <thread>
#include <lib_rt_check.h>

int main()
{
    using namespace std::chrono_literals;

    realtime_context rc;
    std::this_thread::sleep_for (1ns);

    return 0;
}
