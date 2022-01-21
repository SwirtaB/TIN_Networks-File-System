#include <NFSConnection.hpp>

#include <fstream>
#include <vector>
#include <iostream>

extern "C"
{
#include <fcntl.h>
}

std::vector<char> readFile(char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> vec;
    vec.reserve(fileSize);
    vec.insert(
        vec.begin(),
        std::istream_iterator<char>(file),
        std::istream_iterator<char>()
    );
    return vec;
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cout << "copy_file_to_server <file> <hostname> <username> <password> <fs_name> <target>" << std::endl;
        return -1;
    }
    auto file = readFile(argv[1]);

    nfs::NFSConnection connection;
    auto conn = connection.connect(
        argv[2],
        argv[3],
        argv[4],
        argv[5]
    );
    if (conn != nfs::OK) {
        return -1;
    }

    int fd = connection.open(argv[6], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd <= 0) {
        return -1;
    }
    int res = connection.write(fd, file.data(), file.size());
    if (res < 0) {
        return -1;
    }
    std::cout << "Written " << res << " bytes" << std::endl;
    return 0;
}