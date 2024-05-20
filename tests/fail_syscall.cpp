#include <sys/syscall.h>
#include <vector>
#include <lib_rt_check.h>


int main()
{
    realtime_context rc;

    [[maybe_unused]] pid_t tid = syscall(SYS_gettid);

    return 0;
}
