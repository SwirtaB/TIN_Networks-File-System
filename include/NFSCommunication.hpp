#pragma once

#include "NFSMessages.hpp"

namespace nfs
{

    const uint16_t DEFAULT_PORT = 46879;

    int connect_to_server(char *hostname);

    int disconnect_from_server(int descriptor);

    void listen_for_connections(uint16_t port, void (*worker_function)(int));

    void send_message(int descriptor, MSG &message);

    std::unique_ptr<MSG> wait_for_message(int descriptor);
}
