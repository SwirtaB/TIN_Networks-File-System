#pragma once

#include "NFSCommunication.hpp"

#include <string>
#include <unordered_map>

namespace nfs
{
struct NFSServerConfig
{
    std::unordered_map<std::string, std::string> filesystems;
    uint16_t                                     port = DEFAULT_PORT;

    NFSServerConfig() {}
};

class NFSServer
{
  public:
    NFSServer(std::string config_path_ = "/etc/tinnfs.conf");
    int run();

  private:
    std::string     config_path;
    NFSServerConfig config;

    int ensure_running_as_root();
    int load_config();
};

} // namespace nfs