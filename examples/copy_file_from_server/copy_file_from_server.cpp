#include <NFSConnection.hpp>

#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <iterator>

extern "C"
{
#include <fcntl.h>
}

void writeFile(char *filename, char *data, int data_size)
{
    std::ofstream file(filename, std::ios::binary);
    file.write(data, data_size);
}

int main(int argc, char **argv) {if (argc != 7) {
        std::cout << "copy_file_from_server <hostname> <username> <password> <fs_name> <file> <target>" << std::endl;
        return 1;
    }

    nfs::NFSConnection connection;
    auto conn = connection.connect(
        argv[1],
        argv[2],
        argv[3],
        argv[4]
    );
    if (conn != nfs::OK) {
        return 1;
    }

    int fd = connection.open(argv[5], O_RDONLY, 0);
    if (fd <= 0) {
        return 1;
    }
    //find file size with lseek
    auto fileSize = connection.lseek(fd, 0, SEEK_END);
    if (fileSize == (off_t)-1) {
        return 1;
    }
    std::cout << "File size " << fileSize << " bytes" << std::endl;
    //lseek back to beginning
    auto seek_back = connection.lseek(fd, 0, SEEK_SET);
    if (seek_back == (off_t)-1) {
        return 1;
    }
    //creat buffer for read data and read
    char *buffer = new char[fileSize];
    int res = connection.read(fd, buffer, fileSize);
    if (res < 0) {
        std::cerr << connection.get_error() << std::endl;
        return 1;
    }
    int res_close = connection.close(fd);
    if (res_close < 0) {
        return 1;
    }
    writeFile(argv[6], buffer, res);
    delete[] buffer;
    std::cout << "Written " << res << " bytes" << std::endl;
    return 0;
}