#include <thread>
#include <rtcheck.h>

#if __APPLE__
int main()
{
  rtc::realtime_context rc;
  thread_local std::array<std::byte, 10 * 1024 * 1024> ten_megabytes;
  ten_megabytes[0] = std::byte {1};

  return 0;
}
#else
int main()
{
  return 1;
}
#endif
