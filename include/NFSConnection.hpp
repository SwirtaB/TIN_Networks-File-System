#ifndef NFSCONNECTION_HPP
#define NFSCONNECTION_HPP

#include "NFSCommunication.hpp"

namespace nfs
{
class NFSConnection
{
  public:
    NFSConnection(std::string username, std::string password, std::string filesystemName);
    ~NFSConnection() = default;

    int     open(char *path, int oflag, int mode);
    int     close(int fd);
    ssize_t read(int fd, void *buf, size_t count);
    ssize_t write(int fd, const void *buf, size_t count);
    off_t   lseek(int fd, off_t offset, int whence);
    int     fstat(int fd, struct stat *statbyf);
    int     unlink(const char *path);
    int     flock(int fd, int operation);

  private:
    int m_connectionSocket;
};
} // namespace nfs

#endif