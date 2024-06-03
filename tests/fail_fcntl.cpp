#include <rtcheck.h>
#include <memory>
#include <cassert>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <rtcheck.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

int main()
{
    rtc::realtime_context rc;

    namespace fs = std::filesystem;
    const fs::path temp_file = std::tmpnam (nullptr);
    int fd = creat(temp_file.c_str(), S_IRUSR | S_IWUSR);
    assert(fd != -1);

    auto func = [fd] {
        struct flock lock{};
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_pid = ::getpid();

        assert(fcntl(fd, F_GETLK, &lock) == 0);
        assert(lock.l_type == F_UNLCK);
    };

    close(fd);
    std::remove (temp_file.c_str());

    return 0;
}

#pragma clang diagnostic pop
