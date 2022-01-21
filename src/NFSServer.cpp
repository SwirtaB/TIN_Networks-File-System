#include "../include/NFSServerWorker.hpp"

#include <fstream>
#include <iostream>

extern "C"
{
#include <unistd.h>
#include <stdio.h>
}

namespace nfs
{

int NFSServer::run() {
    if (ensure_running_as_root() != 0) return 1;
    if (load_config() != 0) return 1;

    nfs::listen_for_connections(
        [&](int sockfd) { return NFSServerWorker(config, sockfd).run(); },
        config.port
    );
    perror("listening for connections returned");
    return -1;
}

int NFSServer::ensure_running_as_root() {
    if (geteuid() != 0) {
        std::cerr << "Not running as root!" << std::endl;
        return 1;
    }
    return 0;
}

int NFSServer::load_config() {
    std::ifstream config_file("/etc/tinnfs.conf");

    if (!(config_file)) {
        std::cerr << "No config found at /etc/tinnfs.conf" << std::endl;
        return 1;
    }

    std::string option;
    while (config_file >> option) {
        if (option == "port") {
            int64_t port;
            if (!(config_file >> port)) {
                std::cerr << "Error parsing config option: port" << std::endl;
                return 1;
            } else if (port < 0 || port >= 65536) {
                std::cerr << "Error parsing config option port: out of range" << std::endl;
                return 1;
            } else {
                config.port = static_cast<uint16_t>(port);
            }
        } else if (option == "filesystem") {
            std::string name;
            std::string path;
            if (!(config_file >> name)) {
                std::cerr << "Error parsing name of filesystem" << std::endl;
                return 1;
            } else if (!(config_file >> path)) {
                std::cerr << "Error parsing path of filesystem with name " << name << std::endl;
                return 1;
            } else if (!std::filesystem::is_directory(path)) {
                std::cerr << "Filesystem " << name << " does not point to directory" << std::endl;
                return 1;
            } else {
                config.filesystems[name] = path;
                std::cerr << "Loaded filesystem " << name << " at " << path << std::endl;
            }
        } else {
            std::cerr << "Invalid config option: " << option << std::endl;
            return 1;
        }
    }

    if (config.filesystems.empty()) {
        std::cerr << "No filesystems in config" << std::endl;
        return 1;
    }

    return 0;
}

} // namespace nfs