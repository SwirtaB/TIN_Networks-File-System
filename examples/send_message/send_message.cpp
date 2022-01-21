#include "NFSCommunication.hpp"

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) {
        exit(1);
    }
    if (argv[1] == std::string("--client")) {
        int sockfd = nfs::connect_to_server("localhost");
        if (sockfd < 0)
            exit(3);

        std::unique_ptr<nfs::MSG> msg(nullptr);

        int res = nfs::wait_for_message(sockfd, msg);
        if (res < 0)
            exit(4);
        if (res == 0)
            exit(5);
        if (msg == nullptr)
            exit(6);

        nfs::SMSGResultRead *rmsg = dynamic_cast<nfs::SMSGResultRead *>(msg.get());
        if (rmsg == nullptr)
            exit(7);

        std::cout << rmsg->data;

    } else if (argv[1] == std::string("--server")) {
        nfs::listen_for_connections([](int sockfd) -> int {
            const char         *data = "Hello Client!\n";
            uint64_t            size = strlen(data) + 1;
            nfs::SMSGResultRead msg(0, size, data);
            return nfs::send_message(sockfd, msg);
        });

    } else {
        exit(2);
    }
}
