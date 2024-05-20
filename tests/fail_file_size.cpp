#include <filesystem>
#include <lib_rt_check.h>

int main()
{
    std::filesystem::path awk_path ("/usr/bin/awk");
    [[maybe_unused]] std::uintmax_t file_size = std::filesystem::file_size (awk_path);

    return 0;
}
