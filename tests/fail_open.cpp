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

    return 0;
}

#pragma clang diagnostic pop
