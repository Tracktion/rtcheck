#include <lib_rt_check.h>

int main()
{
    realtime_context rc;
    disable_checks_for_thread (check_flags::malloc);

    [[ maybe_unused ]] auto res = malloc (1024);

    return 0;
}
