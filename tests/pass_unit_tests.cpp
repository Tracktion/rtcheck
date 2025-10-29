#include <cassert>
#include <fcntl.h>
#include <filesystem>
#include <rtcheck.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

void test_openCreatesFileWithProperMode()
{
    namespace fs = std::filesystem;
    const fs::path temp_file = fs::temp_directory_path() / "test_file_XXXXXX";

    const int mode = S_IRGRP | S_IROTH | S_IRUSR | S_IWUSR;
    const int fd = open (temp_file.c_str(), O_CREAT | O_WRONLY, mode);
    assert(fd != -1);
    close(fd);

    struct stat st;
    assert(stat(temp_file.c_str(), &st) == 0);

    // Mask st_mode to get permission bits only
    assert((st.st_mode & 0777) == mode);

    auto res = fs::remove (temp_file);
    assert (res);
}

void test_fcntlFlockDiesWhenRealtime()
{
    namespace fs = std::filesystem;
    const fs::path temp_file = fs::temp_directory_path() / "test_file_XXXXXX";
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

    func();

    close(fd);
    std::remove (temp_file.c_str());
}

#pragma clang diagnostic pop

int main()
{
    test_openCreatesFileWithProperMode();
    test_fcntlFlockDiesWhenRealtime();

    return 0;
}

