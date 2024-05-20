#include <filesystem>
#include <thread>
#include <lib_rt_check.h>

void do_file_read();

int main()
{
std::thread t1 ([] {
    realtime_context rc;
    do_file_read();
});
    t1.join();
//    std::filesystem::path awk_path ("/usr/bin/awk");
//    [[maybe_unused]] std::uintmax_t file_size = std::filesystem::file_size (awk_path);

    return 0;
}

void do_file_read()
{
    std::filesystem::path awk_path ("/usr/bin/awk");
    [[maybe_unused]] std::uintmax_t file_size = std::filesystem::file_size (awk_path);
}
