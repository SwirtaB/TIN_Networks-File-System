#pragma once

#include "NFSCommunication.hpp"

namespace nfs
{

    void worker_function(int client_socket);

    class ServerWorker
    {
    public:
        ServerWorker(int client_socket_);
        void run();

    private:
        int client_socket;
    };
}