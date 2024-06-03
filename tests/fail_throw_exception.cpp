#include <stdexcept>
#include <rtcheck.h>

int main()
{
    rtc::realtime_context rc;

    try
    {
        throw (std::runtime_error ("runtime_error"));
    }
    catch (std::runtime_error)
    {
    }

    return 0;
}
