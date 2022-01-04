#pragma once

#include "NFSMessages.hpp"

namespace nfs
{

    const uint16_t DEFAULT_PORT = 46879;

    // Returns a connection descriptor or error if less than zero.
    int connect_to_server(char *hostname);

    // Closes tcp connection and invalidates the descriptor.
    void disconnect_from_server(int descriptor);

    // Listen for connections on given port and spawn run the given
    // worker function on a separate thread for every connection.
    void listen_for_connections(uint16_t port, void (*worker_function)(int));

    // Pass any subclass of MSG to send it.
    void send_message(int descriptor, MSG &message);

    // Use msg->code to determine what kind of message it is,
    // and then reinterpret_cast or dynamic_cast to it.
    std::unique_ptr<MSG> wait_for_message(int descriptor);
}
