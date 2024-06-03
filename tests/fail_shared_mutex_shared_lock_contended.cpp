#include <shared_mutex>
#include <thread>
#include <rtcheck.h>

void sleep_non_realtime()
{
    using namespace std::chrono_literals;

    rtc::non_realtime_context nrc;
    std::this_thread::sleep_for (1s);
}

int main()
{
    std::shared_mutex m;

    std::thread t1 ([&]
                    {
                        rtc::realtime_context rc;

                        std::shared_lock l (m);
                        sleep_non_realtime();
                    });

    std::thread t2 ([&]
                    {
                        rtc::realtime_context rc;

                        std::shared_lock l (m);
                        sleep_non_realtime();
                    });
    t1.join();
    t2.join();

    return 0;
}
