#include <lib_rt_check.h>

int main()
{
    realtime_context rc;
    disable_checks_for_thread (check_flags::memory);

    auto res = malloc (1024);
    res = realloc(res, 1024 * 2);
    free(res);

    return 0;
}
