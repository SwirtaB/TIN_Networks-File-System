#pragma once

#include "NFSMessages.hpp"

namespace nfs
{

    const uint16_t DEFAULT_PORT = 46879;
    const int SERVER_QUEUE_LIMIT = 10;

    // Returns a TCP connection socket descriptor or error if less than zero.
    int connect_to_server(const char *hostname, uint16_t port = DEFAULT_PORT);

    // Closes the TCP socket and invalidates the descriptor.
    int disconnect_from_server(int descriptor);

    // Listen for connections on given port and fork with the given worker function for every connection.
    int listen_for_connections(void (*worker_function)(int), uint16_t port = DEFAULT_PORT, int queue_limit = SERVER_QUEUE_LIMIT);

    // Pass any subclass of MSG to send it.
    // Returns number of bytes sent. 0 means the TCP connection is closed, < 0 means error.
    int send_message(int descriptor, MSG &message);

    // Places the received message in msg_ptr.
    // Use msg->code to determine what kind of message it is and then dynamic_cast to it.
    // Returns number of bytes received. 0 means the TCP connection is closed, < 0 means error.
    int wait_for_message(int descriptor, std::unique_ptr<MSG> &msg_ptr);
}
