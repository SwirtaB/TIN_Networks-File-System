#ifndef NFSCONNECTION_HPP
#define NFSCONNECTION_HPP

#include "NFSCommunication.hpp"

#include <string>

namespace nfs
{

/**
 * @brief Return codes enum for establishing connection and authentication.
 * After a successful connection, standard errno codes are used.
 *
 */
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
    /**
     * @brief Construct a new NFSConnection object.
     * Object do not establish connection by itself.
     */
    NFSConnection();

    /**
     * @brief Destroy the NFSConnection object.
     * If object do own socket it tries to send to server disconnect message.
     * If sending message failes 3 times, we assume that there is problem with connection and object is distroied.
     */
    ~NFSConnection();

    /**
     * @brief Tries establish connection to filesystem on given server.
     *
     * @param hostName server name
     * @param username user name (for authentication)
     * @param password password (for authentication)
     * @param filesystemName name of filesystem we want to get access to
     * @return ConnectReturn return code, @see nfs::ConnectReturn
     */
    ConnectReturn connect(const std::string &hostName,
                          const std::string &username,
                          const std::string &password,
                          const std::string &filesystemName);

    /**
     * @brief Wrapps standar open(2) function. Interface is the same.
     *
     * @param path
     * @param oflag
     * @param mode
     * @return int
     */
    int open(char *path, int oflag, int mode);

    /**
     * @brief Wrapps standar close(2) function. Interface is the same.
     *
     * @param fd
     * @return int
     */
    int close(int fd);

    /**
     * @brief Wrapps standar read(2) function. Interface is the same.
     *
     * @param fd
     * @param buf
     * @param count
     * @return ssize_t
     */
    ssize_t read(int fd, void *buf, size_t count);

    /**
     * @brief Wrapps standar write(2) function. Interface is the same.
     *
     * @param fd
     * @param buf
     * @param count
     * @return ssize_t
     */
    ssize_t write(int fd, const void *buf, size_t count);

    /**
     * @brief Wrapps standar lseek(2) function. Interface is the same.
     *
     * @param fd
     * @param offset
     * @param whence
     * @return off_t
     */
    off_t lseek(int fd, off_t offset, int whence);

    /**
     * @brief Wrapps standar fstat(3) function. Interface is the same.
     *
     * @param fd
     * @param statbuf
     * @return int
     */
    int fstat(int fd, struct stat *statbuf);

    /**
     * @brief Wrapps standar unlink(2) function. Interface is the same.
     *
     * @param path
     * @return int
     */
    int unlink(const char *path);

    /**
     * @brief Wrapps standar flock(2) function. Interface is the same.
     *
     * @param fd
     * @param operation
     * @return int
     */
    int flock(int fd, int operation);

    /**
     * @brief Returns errno stored in object, upon creation it is 0.
     * Stored errno value is not cleard at any time, check it only if function not finished successfully.
     * @return int64_t errno code
     */
    int64_t get_error();

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