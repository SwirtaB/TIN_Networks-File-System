#include "NFSServer.hpp"

#include <iostream>

int main(int argc, char **argv) {
    if (argc == 1) {
        return nfs::NFSServer().run();
    } else if (argc == 2) {
        return nfs::NFSServer(argv[1]).run();
    } else {
        std::cout << "nfs_server [config_path]" << std::endl;
        return 1;
    }
}