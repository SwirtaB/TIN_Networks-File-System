#include "../include/NFSServerWorker.hpp"

namespace nfs
{

void worker_function(int client_socket) {
    ServerWorker(client_socket).run();
}

ServerWorker::ServerWorker(int client_socket_) : client_socket(client_socket_) {}

void ServerWorker::run() {
    // TODO
}
} // namespace nfs