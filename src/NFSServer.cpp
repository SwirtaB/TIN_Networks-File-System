#include "../include/NFSServer.hpp"
#include "../include/NFSServerWorker.hpp"

namespace nfs
{
    void NFSServer::run(uint16_t port)
    {
        nfs::listen_for_connections(worker_function, port);
    }
}