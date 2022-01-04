#pragma once

#include "NFSCommunication.hpp"

namespace nfs
{

    void worker_function(int descriptor);

    class ServerWorker
    {
    public:
        ServerWorker(int descriptor_);
        void run();

    private:
        int descriptor;
    };
}