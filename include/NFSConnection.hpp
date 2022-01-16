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
    ~NFSConnection() = default;

    ConnectReturn connect(const std::string &hostName,
                          const std::string &username,
                          const std::string &password,
                          const std::string &filesystemName);
    int           open(char *path, int oflag, int mode);
    int           close(int fd);
    ssize_t       read(int fd, void *buf, size_t count);
    ssize_t       write(int fd, const void *buf, size_t count);
    off_t         lseek(int fd, off_t offset, int whence);
    int           fstat(int fd, struct stat *statbyf);
    int           unlink(const char *path);
    int           flock(int fd, int operation);

  private:
    ConnectReturn log_in(const std::string &username, const std::string &password);
    ConnectReturn send_username(const std::string &username);
    ConnectReturn send_password(const std::string &password);
    ConnectReturn access_filesystem(const std::string filesystemName);

    bool m_access;
    int  m_sockfd;
};
} // namespace nfs

#endif