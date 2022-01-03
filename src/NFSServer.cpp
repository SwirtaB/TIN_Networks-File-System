#include "../include/NFSServer.hpp"
#include "../include/NFSServerWorker.hpp"

namespace nfs
{
    void NFSServer::run(uint16_t port = nfs::DEFAULT_PORT)
    {
        nfs::listen_for_connections(port, nfs::worker_function);
    }
}