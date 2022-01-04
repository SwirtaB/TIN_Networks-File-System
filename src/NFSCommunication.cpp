#include "../include/NFSCommunication.hpp"

namespace nfs
{

    int connect_to_server(char *hostname)
    {
        // TODO: tcp connect and return socket descriptor
    }

    int disconnect_from_server(int descriptor)
    {
        // TODO: tcp disconnect
    }

    void listen_for_connections(uint16_t port, void (*worker_function)(int))
    {
        // TODO: listen for tcp connection and fork
        int socket_descriptor = 0; // temp

        // forked:
        worker_function(socket_descriptor);
        disconnect_from_server(socket_descriptor);
    }

    void send_message(int descriptor, MSG &message)
    {
        // TODO: send data to socket
    }

    std::unique_ptr<MSG> wait_for_message(int descriptor)
    {
        // TODO: wait on socket read for message
    }
}