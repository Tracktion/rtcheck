#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include <lib_rt_check.h>


int main()
{
    realtime_context rc;

    namespace fs = std::filesystem;

    if (fs::path file_path ("/usr/bin/awk");
        fs::exists (file_path))
    {
        // Open the file in binary mode
        const auto file_size = fs::file_size (file_path);
        std::vector<char> file_data (file_size);

        // Read the file into the vector
        std::ifstream file (file_path, std::ios::binary);
        file.read (file_data.data(), file_size);

        // Check if the file was read successfully
        if (file)
            std::cout << "File read successfully." << std::endl;
        else
            std::cout << "Error reading the file." << std::endl;

        file.close();
    }
    else
    {
        std::cout << "File does not exist." << std::endl;
    }

    return 0;
}
