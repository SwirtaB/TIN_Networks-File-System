#include <NFSConnection.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sys/stat.h>
#include <vector>

extern "C"
{
#include <fcntl.h>
}

std::vector<char> readFile(char *filename) {
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> vec;
    vec.reserve(fileSize);
    vec.insert(vec.begin(), std::istream_iterator<char>(file), std::istream_iterator<char>());
    return vec;
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cout << "copy_file_to_server <file> <hostname> <username> <password> <fs_name> <target>" << std::endl;
        return -1;
    }
    auto file = readFile(argv[1]);

    nfs::NFSConnection connection;
    auto               conn = connection.connect(argv[2], argv[3], argv[4], argv[5]);
    if (conn != nfs::OK) {
        return -1;
    }

    int fd = connection.open(argv[6], O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd <= 0) {
        std::cout << "open " << connection.get_error() << std::endl;
        return -1;
    }
    int res = connection.write(fd, file.data(), file.size());
    if (res < 0) {
        std::cout << "write " << connection.get_error() << std::endl;
        return -1;
    }
    int res_close = connection.close(fd);
    if (res_close < 0) {
        std::cout << "close " << connection.get_error() << std::endl;
        return -1;
    }
    std::cout << "Written " << res << " bytes" << std::endl;
    return 0;
}
