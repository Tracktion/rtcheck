#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <lib_rt_check.h>

int main()
{
    int fd = open("/usr/bin/awk", O_RDONLY);

    if (fd == -1)
    {
        std::cout << "ERROR: open failed\n";
    	return 0;
    }

    struct stat s;
    int status = fstat (fd, &s);
    auto size = s.st_size;

    char* map = nullptr;

    {
        rtc::realtime_context rc;
        map = (char *) mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    }

    if (map == MAP_FAILED)
    {
        std::cout << "ERROR: MAP_FAILED\n";
        return 0;
    }

    if (munmap(map, size) == -1)
    {
        std::cout << "ERROR: munmap failed\n";
        return 0;
    }

    close(fd);

    return 0;
}
