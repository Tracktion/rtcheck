#include <mutex>
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
    std::mutex m;

    std::thread t1 ([&]
                    {
                        rtc::realtime_context rc;

                        if (std::unique_lock l (m, std::try_to_lock); l.owns_lock())
                            sleep_non_realtime();
                    });
    t1.join();

    std::thread t2 ([&]
                    {
                        rtc::realtime_context rc;

                        if (std::unique_lock l (m, std::try_to_lock); l.owns_lock())
                            sleep_non_realtime();
                    });
    t2.join();

    return 0;
}
