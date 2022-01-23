#include "../include/NFSCommunication.hpp"

#include <functional>

extern "C"
{
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

#ifdef __unix__
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#define ntohll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#endif

namespace nfs
{

int connect_to_server(const char *hostname, uint16_t port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("connect_to_server: Error opening socket");
        return sock;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);

    struct hostent *server_hostent = gethostbyname(hostname);
    if (server_hostent == 0) {
        perror("connect_to_server: Error getting host by name");
        return -1;
    }
    memcpy(&server_addr.sin_addr, server_hostent->h_addr, server_hostent->h_length);

    int connect_res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_res < 0) {
        perror("connect_to_server: Error connecting to server");
        return connect_res;
    }

    return sock;
}

int disconnect_from_server(int descriptor) {
    return close(descriptor);
}

int listen_for_connections(std::function<int(int)> worker_function, uint16_t port, int queue_limit) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("listen_for_connections: Error opening socket");
        return sock;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int bind_res = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_res < 0) {
        perror("listen_for_connections: Error binding");
        return bind_res;
    }

    while (true) {
        if (listen(sock, queue_limit) == 0) {
            int client_sock = accept(sock, nullptr, nullptr);
            if (client_sock < 0) {
                perror("listen_for_connections: Error accepting connection");
                return client_sock;
            };

            if (fork() == 0) { // child
                close(sock);
                int res = worker_function(client_sock);
                exit(res);
            } else { // parent
                close(client_sock);
            }
        } else {
            return -1;
        }
    }
}

int send_message(int descriptor, MSG &message) {
    // make message buffer
    MessageBuffer buffer;
    message.push_to_buffer(buffer);

    // send message size
    int64_t message_size = htonll(buffer.size());
    char    message_size_buff[sizeof(message_size)];
    memcpy(message_size_buff, &message_size, sizeof(message_size));
    size_t size_sent = 0;
    while (static_cast<size_t>(size_sent) < sizeof(message_size)) {
        ssize_t res = send(descriptor, message_size_buff + size_sent, sizeof(message_size) - size_sent, 0);
        if (res <= 0) {
            return res;
        } else {
            size_sent += res;
        }
    }

    // send message
    ssize_t to_send = buffer.size();
    ssize_t sent    = 0;
    while (sent < to_send) {
        ssize_t res = send(descriptor, buffer.data() + sent, to_send - sent, 0);
        if (res <= 0) {
            return res;
        } else {
            sent += res;
        }
    }
    return size_sent + sent;
}

int wait_for_message(int descriptor, std::unique_ptr<MSG> &msg_ptr) {
    // receive size
    char   size_buffer[sizeof(int64_t)];
    size_t received_size = 0;
    while (received_size < sizeof(int64_t)) {
        int received = recv(descriptor, size_buffer + received_size, sizeof(int64_t) - received_size, 0);
        if (received <= 0) {
            return received;
        } else {
            received_size += received;
        }
    }
    uint64_t size = ntohll(*reinterpret_cast<int64_t *>(size_buffer));

    // receive message
    char  *message_buffer   = new char[size];
    size_t received_message = 0;
    while (received_message < size) {
        int received = recv(descriptor, message_buffer + received_message, size - received_message, 0);
        if (received <= 0) {
            return received;
        } else {
            received_message += received;
        }
    }
    MessageBuffer msgbuff(message_buffer, size);
    delete[](message_buffer);

    // make message
    MSGCode code = static_cast<MSGCode>(msgbuff.data()[0]);
    MSG    *msg;
    switch (code) {
        case MSGCode::CONNECT_INFO_USERNAME:
            msg = CMSGConnectInfoUsername::from_buffer(msgbuff);
            break;
        case MSGCode::CONNECT_INFO_PASSWORD:
            msg = CMSGConnectInfoPassword::from_buffer(msgbuff);
            break;
        case MSGCode::CONNECT_INFO_FSNAME:
            msg = CMSGConnectInfoFSName::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_OPEN:
            msg = CMSGRequestOpen::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_CLOSE:
            msg = CMSGRequestClose::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_READ:
            msg = CMSGRequestRead::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_WRITE:
            msg = CMSGRequestWrite::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_LSEEK:
            msg = CMSGRequestLseek::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_FSTAT:
            msg = CMSGRequestFstat::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_UNLINK:
            msg = CMSGRequestUnlink::from_buffer(msgbuff);
            break;
        case MSGCode::REQUEST_FLOCK:
            msg = CMSGRequestFlock::from_buffer(msgbuff);
            break;
        case MSGCode::DISCONNECT:
            msg = new CMSGDisconnect;
            break;
        case MSGCode::PROVIDE_USERNAME:
            msg = new SMSGProvideUsername;
            break;
        case MSGCode::PROVIDE_PASSWORD:
            msg = new SMSGProvidePassword;
            break;
        case MSGCode::PROVIDE_FSNAME:
            msg = new SMSGProvideFSName;
            break;
        case MSGCode::AUTHORIZATION_OK:
            msg = new SMSGAuthorizationOk;
            break;
        case MSGCode::AUTHORIZATION_FAILED:
            msg = new SMSGAuthorizationFailed;
            break;
        case MSGCode::RESULT_OPEN:
            msg = SMSGResultOpen::from_buffer(msgbuff);
            break;
        case MSGCode::RESULT_CLOSE:
            msg = SMSGResultClose::from_buffer(msgbuff);
            break;
        case MSGCode::RESULT_READ:
            msg = SMSGResultRead::from_buffer(msgbuff);
            break;
        case MSGCode::RESULT_WRITE:
            msg = SMSGResultWrite::from_buffer(msgbuff);
            break;
        case MSGCode::RESULT_LSEEK:
            msg = SMSGResultLseek::from_buffer(msgbuff);
            break;
        case MSGCode::RESULT_FSTAT:
            msg = SMSGResultFstat::from_buffer(msgbuff);
            break;
        case MSGCode::RESULT_UNLINK:
            msg = SMSGResultUnlink::from_buffer(msgbuff);
            break;
        case MSGCode::RESULT_FLOCK:
            msg = SMSGResultFlock::from_buffer(msgbuff);
            break;
        default:
            exit(123);
    }

    msg_ptr.reset(msg);
    return received_size + received_message;
}
} // namespace nfs