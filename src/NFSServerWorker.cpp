#include "../include/NFSServerWorker.hpp"

namespace nfs
{

    void worker_function(int descriptor)
    {
        ServerWorker(descriptor).run();
    }

    ServerWorker::ServerWorker(int descriptor_) : descriptor(descriptor_) {}

    void ServerWorker::run()
    {
        // TODO
    }
}