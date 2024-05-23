#include <mutex>
#include <thread>
#include <lib_rt_check.h>

void sleep_non_realtime()
{
    using namespace std::chrono_literals;

    non_realtime_context nrc;
    std::this_thread::sleep_for (1s);
}

int main()
{
    std::mutex m;

    std::thread t1 ([&]
                    {
                        realtime_context rc;

                        std::unique_lock l (m);
                        sleep_non_realtime();
                    });
    t1.join();

    std::thread t2 ([&]
                    {
                        realtime_context rc;

                        std::unique_lock l (m);
                        sleep_non_realtime();
                    });
    t2.join();

    return 0;
}