#pragma once

#include "NFSCommunication.hpp"

#include <unordered_map>
#include <string>

namespace nfs
{
struct NFSServerConfig {
    std::unordered_map<std::string, std::string> filesystems;
    uint16_t port = DEFAULT_PORT;

    NFSServerConfig() {}
};

class NFSServer
{
public:
    int run();

private:
    NFSServerConfig config;

    int ensure_running_as_root();
    int load_config();
};

} // namespace nfs