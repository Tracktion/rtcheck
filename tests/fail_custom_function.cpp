#include <lib_rt_check.h>

void my_func()
{
    log_function_if_realtime_context (__func__);
}

int main()
{
    my_func();

    realtime_context rc;
    my_func();

    return 0;
}
