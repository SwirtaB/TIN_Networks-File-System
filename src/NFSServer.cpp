#include "../include/Logging.hpp"
#include "../include/NFSServerWorker.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

extern "C"
{
#include <stdio.h>
#include <unistd.h>
}

namespace nfs
{

NFSServer::NFSServer(std::string config_path_) : config_path(config_path_) {}

int NFSServer::run() {
    if (ensure_running_as_root() != 0)
        return 1;
    if (load_config() != 0)
        return 1;

    nfs::listen_for_connections(
        [&](int sockfd) {
            int res = NFSServerWorker(config, sockfd).run();
            log_cerr("Server worker finished with code ", res);
            return res;
        },
        config.port);
    log_cerr("Server shutting down");
    return -1;
}

int NFSServer::ensure_running_as_root() {
    if (geteuid() != 0) {
        log_cerr("Not running as root!");
        return 1;
    }
    return 0;
}

int NFSServer::load_config() {
    std::ifstream config_file(config_path);

    if (!(config_file)) {
        log_cerr("No config found at ");
        return 1;
    }

    std::string option;
    while (config_file >> option) {
        if (option == "port") {
            int64_t port;
            if (!(config_file >> port)) {
                log_cerr("Error parsing config option: port");
                return 1;
            } else if (port < 0 || port >= 65536) {
                log_cerr("Error parsing config option port: out of range");
                return 1;
            } else {
                config.port = static_cast<uint16_t>(port);
            }
        } else if (option == "filesystem") {
            std::string name;
            std::string path;
            if (!(config_file >> name)) {
                log_cerr("Error parsing name of filesystem");
                return 1;
            } else if (!(config_file >> path)) {
                log_cerr("Error parsing path of filesystem with name ", name);
                return 1;
            } else if (!std::filesystem::is_directory(path)) {
                log_cerr("Filesystem ", name, " does not point to directory");
                return 1;
            } else {
                config.filesystems[name] = path;
                log_cerr("Loaded filesystem ", name, " at ", path);
            }
        } else {
            log_cerr("Invalid config option: ", option);
            return 1;
        }
    }

    if (config.filesystems.empty()) {
        log_cerr("No filesystems in config");
        return 1;
    }

    return 0;
}

} // namespace nfs