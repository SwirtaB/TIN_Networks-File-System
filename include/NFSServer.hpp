#pragma once

#include "NFSCommunication.hpp"

namespace nfs
{

    class NFSServer
    {
    public:
        void run(uint16_t port = nfs::DEFAULT_PORT);
    };
}