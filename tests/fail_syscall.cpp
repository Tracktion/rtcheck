#include <unistd.h>
#include <sys/syscall.h>
#include <rtcheck.h>


int main()
{
    rtc::realtime_context rc;

    #pragma clang diagnostic push
    // syscall is deprecated, but still in use in libc++
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"

    [[maybe_unused]] pid_t tid = syscall(SYS_gettid);

    #pragma clang diagnostic pop

    return 0;
}
