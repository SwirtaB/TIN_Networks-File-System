#ifndef NFSCONNECTION_HPP
#define NFSCONNECTION_HPP

#include "NFSCommunication.hpp"

#include <string>

namespace nfs
{

enum ConnectReturn
{
    TCP_ERROR,
    INVALID_HOST_NAME,
    SERVER_NOT_RESPONDING,
    INVALID_SERVER_REQUEST,
    LOGIN_FAILED,
    ACCESS_DENIED,
    OK
};

class NFSConnection
{
  public:
    NFSConnection();
    ~NFSConnection();

    ConnectReturn connect(const std::string &hostName,
                          const std::string &username,
                          const std::string &password,
                          const std::string &filesystemName);
    int           open(char *path, int oflag, int mode);        // FIXME use helper method send_and_wait
    int           close(int fd);                                // FIXME use helper method send_and_wait
    ssize_t       read(int fd, void *buf, size_t count);        // FIXME use helper method send_and_wait
    ssize_t       write(int fd, const void *buf, size_t count); // FIXME use helper method send_and_wait
    off_t         lseek(int fd, off_t offset, int whence);      // FIXME use helper method send_and_wait
    int           fstat(int fd, struct stat *statbyf);          // TODO fstat
    int           unlink(const char *path);                     // TODO unlink
    int           flock(int fd, int operation);                 // TODO flock
    int64_t       get_error();

  private:
    ConnectReturn log_in(const std::string &username, const std::string &password);
    ConnectReturn send_username(const std::string &username);
    ConnectReturn send_password(const std::string &password);
    ConnectReturn access_filesystem(const std::string filesystemName);

    int send_and_wait(nfs::MSG &clientMessage, std::unique_ptr<MSG> &resultMessage_ptr);

    bool    m_access;
    int     m_sockfd;
    int64_t m_errno;
};
} // namespace nfs

#endif