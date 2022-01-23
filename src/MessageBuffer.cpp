#include "../include/MessageBuffer.hpp"

#include <cstring>

extern "C"
{
#include <netinet/in.h>
}

#ifdef __unix__
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#define ntohll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#endif

namespace nfs
{
MessageBuffer::MessageBuffer() {}

MessageBuffer::MessageBuffer(char *data, uint64_t data_size) : buffer(data, data + data_size) {}

char *MessageBuffer::data() {
    return buffer.data();
}

size_t MessageBuffer::size() {
    return buffer.size();
}

void MessageBuffer::push_uint8_t(uint8_t code) {
    buffer.push_back(code);
}

void MessageBuffer::push_int64_t(int64_t data) {
    int64_t net = htonll(data);
    char    buff[sizeof(net)];
    memcpy(&buff, &net, sizeof(net));
    for (size_t i = 0; i < sizeof(data); ++i)
        buffer.push_back(buff[i]);
}

int64_t MessageBuffer::pop_int64_t() {
    char  buff[sizeof(int64_t)];
    char *data_pointer = buffer.data() + (buffer.size() - sizeof(int64_t));
    memcpy(&buff, data_pointer, sizeof(int64_t));
    for (size_t i = 0; i < sizeof(int64_t); ++i)
        buffer.pop_back();
    return ntohll(*reinterpret_cast<int64_t *>(buff));
}

void MessageBuffer::push_uint64_t(uint64_t data) {
    uint64_t net = htonll(data);
    char     buff[sizeof(net)];
    memcpy(&buff, &net, sizeof(net));
    for (size_t i = 0; i < sizeof(data); ++i)
        buffer.push_back(buff[i]);
}

uint64_t MessageBuffer::pop_uint64_t() {
    char  buff[sizeof(uint64_t)];
    char *data_pointer = buffer.data() + (buffer.size() - sizeof(uint64_t));
    memcpy(&buff, data_pointer, sizeof(uint64_t));
    for (size_t i = 0; i < sizeof(uint64_t); ++i)
        buffer.pop_back();
    return ntohll(*reinterpret_cast<uint64_t *>(buff));
}

void MessageBuffer::push_char_data(char *data, uint64_t data_size) {
    for (uint64_t i = 0; i < data_size; ++i) {
        buffer.push_back(data[i]);
    }
}

std::vector<char> MessageBuffer::pop_char_data(uint64_t data_size) {
    std::vector<char> buff;
    char             *data_pointer = buffer.data() + (buffer.size() - data_size);
    for (uint64_t i = 0; i < data_size; ++i) {
        buff.push_back(data_pointer[i]);
    }
    for (uint64_t i = 0; i < data_size; ++i) {
        buffer.pop_back();
    }
    return buff;
}

void MessageBuffer::push_stat(struct stat &statbuf) {
    push_int64_t(statbuf.st_dev);
    push_int64_t(statbuf.st_ino);
    push_int64_t(statbuf.st_mode);
    push_int64_t(statbuf.st_nlink);
    push_int64_t(statbuf.st_uid);
    push_int64_t(statbuf.st_gid);
    push_int64_t(statbuf.st_rdev);
    push_int64_t(statbuf.st_size);
    push_int64_t(statbuf.st_blksize);
    push_int64_t(statbuf.st_blocks);
    push_int64_t(statbuf.st_atime);
    push_int64_t(statbuf.st_mtime);
    push_int64_t(statbuf.st_ctime);
}

struct stat MessageBuffer::pop_stat()
{
    struct stat statbuf;
    statbuf.st_ctime   = pop_int64_t();
    statbuf.st_mtime   = pop_int64_t();
    statbuf.st_atime   = pop_int64_t();
    statbuf.st_blocks  = pop_int64_t();
    statbuf.st_blksize = pop_int64_t();
    statbuf.st_size    = pop_int64_t();
    statbuf.st_rdev    = pop_int64_t();
    statbuf.st_gid     = pop_int64_t();
    statbuf.st_uid     = pop_int64_t();
    statbuf.st_nlink   = pop_int64_t();
    statbuf.st_mode    = pop_int64_t();
    statbuf.st_ino     = pop_int64_t();
    statbuf.st_dev     = pop_int64_t();
    return statbuf;
}
} // namespace nfs