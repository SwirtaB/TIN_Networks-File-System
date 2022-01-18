#include "NFSConnection.hpp"

#include "NFSCommunication.hpp"
#include "NFSMessages.hpp"

#include <cstdint>
#include <cstring>
#include <string>

namespace nfs
{

NFSConnection::NFSConnection() : m_access(false), m_sockfd(-1) {}

NFSConnection::~NFSConnection() {
    if (m_sockfd >= 0) {
        // TODO: czy próba retransmisji wiadomości disconnect to dobry pomysł?
        nfs::CMSGDisconnect msg;
        int                 repeat = 3;
        while (nfs::send_message(m_sockfd, msg) <= 0 || repeat != 0) {
            --repeat;
        }
    }
}

ConnectReturn NFSConnection::connect(const std::string &hostName,
                                     const std::string &username,
                                     const std::string &password,
                                     const std::string &filesystemName) {
    m_sockfd = nfs::connect_to_server(hostName.c_str());
    if (m_sockfd < 0)
        return nfs::INVALID_HOST_NAME;

    auto result = log_in(username, password);
    if (result != nfs::OK)
        return result;

    result = access_filesystem(filesystemName);
    if (result != nfs::OK)
        return result;

    m_access = true;
    return nfs::OK;
}

int NFSConnection::open(char *path, int oflag, int mode) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return -1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestOpen      cmsg(oflag, mode, std::strlen(path), path);

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return -1;
    }

    // Rzutowanie widaomości na spodziewany typ w celu odczytania zawartości.
    // Obsługa błędu rzutowania - spodziewano się innej wiadomości
    nfs::SMSGResultOpen *rmsg = dynamic_cast<nfs::SMSGResultOpen *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    // Odczytanie informacji z wiadomości i zwrócenie ich jako wyniku funkcji.
    auto fd = rmsg->fd;
    if (fd < 0) {
        m_errno = rmsg->_errno;
        return -1;
    }

    return fd;
}

int NFSConnection::close(int fd) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return -1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestClose     cmsg(fd);

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return -1;
    }

    // Rzutowanie widaomości na spodziewany typ w celu odczytania zawartości.
    // Obsługa błędu rzutowania - spodziewano się innej wiadomości.
    nfs::SMSGResultClose *rmsg = dynamic_cast<nfs::SMSGResultClose *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    // Odczytanie informacji z wiadomości i zwrócenie ich jako wyniku funkcji.
    if (rmsg->result < 0) {
        m_errno = rmsg->_errno;
        return -1;
    }

    return 0;
}

ssize_t NFSConnection::read(int fd, void *buf, size_t count) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return -1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestRead      cmsg(fd, count);

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return -1;
    }

    // Rzutowanie widaomości na spodziewany typ w celu odczytania zawartości.
    // Obsługa błędu rzutowania - spodziewano się innej wiadomości.
    nfs::SMSGResultRead *rmsg = dynamic_cast<nfs::SMSGResultRead *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    // Sprawdzenie czy nie wystąpił błąd.
    if (rmsg->_errno != 0) {
        m_errno = rmsg->_errno;
        return -1;
    }

    // Przekazanie wartości do bufora i zwrócenie ilości odczytanych znaków.
    buf = rmsg->data;
    return rmsg->data_size;
}

ssize_t NFSConnection::write(int fd, const void *buf, size_t count) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return -1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestWrite     cmsg(fd, count, static_cast<const char *>(buf));

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return -1;
    }

    // Rzutowanie widaomości na spodziewany typ w celu odczytania zawartości.
    // Obsługa błędu rzutowania - spodziewano się innej wiadomości.
    nfs::SMSGResultWrite *rmsg = dynamic_cast<nfs::SMSGResultWrite *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    // Sprawdzenie czy nie wystąpił błąd.
    if (rmsg->_errno != 0) {
        m_errno = rmsg->_errno;
        return -1;
    }

    return rmsg->result;
}

off_t NFSConnection::lseek(int fd, off_t offset, int whence) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return (off_t)-1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestLseek     cmsg(fd, offset, whence);

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return (off_t)-1;
    }

    // Rzutowanie widaomości na spodziewany typ w celu odczytania zawartości.
    // Obsługa błędu rzutowania - spodziewano się innej wiadomości.
    nfs::SMSGResultLseek *rmsg = dynamic_cast<nfs::SMSGResultLseek *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return (off_t)-1;
    }

    // Sprawdzenie czy nie wystąpił błąd.
    if (rmsg->_errno != 0) {
        m_errno = rmsg->_errno;
        return (off_t)-1;
    }

    return rmsg->offset;
}

int NFSConnection::fstat(int fd, struct stat *statbuf) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return -1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestFstat     cmsg(fd);

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return -1;
    }

    // Rzutowanie widaomości na spodziewany typ w celu odczytania zawartości.
    // Obsługa błędu rzutowania - spodziewano się innej wiadomości.
    nfs::SMSGResultFstat *rmsg = dynamic_cast<nfs::SMSGResultFstat *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    // Sprawdzenie czy nie wystąpił błąd i ustawienie flagi errno.
    if (rmsg->result != 0) {
        m_errno = rmsg->_errno;
        return -1;
    }

    *statbuf = rmsg->statbuf;
    return 0;
}

int NFSConnection::unlink(const char *path) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return -1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestUnlink    cmsg(std::strlen(path), path);

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return -1;
    }

    nfs::SMSGResultUnlink *rmsg = dynamic_cast<nfs::SMSGResultUnlink *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    // Sprawdzenie czy nie wystąpił błąd i ustawienie flagi errno.
    if (rmsg->result != 0) {
        m_errno = rmsg->_errno;
        return -1;
    }

    // Successfully finished.
    return 0;
}

int NFSConnection::flock(int fd, int operation) {
    // Weryfikacja czy mamy zestawione połączenie i autoryzację.
    if (!m_access) {
        m_errno = EACCES;
        return -1;
    }

    std::unique_ptr<nfs::MSG> msg(nullptr);
    nfs::CMSGRequestFlock     cmsg(fd, operation);

    // m_errno ustawiane przez metodę send_and_wait.
    if (send_and_wait(cmsg, msg) < 0) {
        return -1;
    }

    nfs::SMSGResultFlock *rmsg = dynamic_cast<nfs::SMSGResultFlock *>(msg.get());
    if (rmsg == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    if (rmsg->result != 0) {
        m_errno = rmsg->_errno;
        return -1;
    }

    return 0;
}

int64_t NFSConnection::get_error() {
    return m_errno;
}

ConnectReturn NFSConnection::log_in(const std::string &username, const std::string &password) {
    auto result = send_username(username);
    if (result != nfs::OK)
        return result;

    result = send_password(password);
    if (result != nfs::OK)
        return result;

    return nfs::OK;
}

ConnectReturn NFSConnection::send_username(const std::string &username) {
    std::unique_ptr<nfs::MSG> msg(nullptr);

    int result = nfs::wait_for_message(m_sockfd, msg);
    if (result <= 0 || msg == nullptr)
        return nfs::SERVER_NOT_RESPONDING;

    nfs::SMSGProvideUsername *rmsg = dynamic_cast<nfs::SMSGProvideUsername *>(msg.get());
    if (rmsg == nullptr)
        return nfs::INVALID_SERVER_REQUEST;

    nfs::CMSGConnectInfoUsername cmsg(username.size(), username.c_str());
    if (nfs::send_message(m_sockfd, cmsg) < username.size())
        return nfs::TCP_ERROR;

    return nfs::OK;
}

ConnectReturn NFSConnection::send_password(const std::string &password) {
    std::unique_ptr<nfs::MSG> msg(nullptr);

    int result = nfs::wait_for_message(m_sockfd, msg);
    if (result <= 0 || msg == nullptr)
        return nfs::SERVER_NOT_RESPONDING;

    nfs::SMSGProvidePassword *rmsg = dynamic_cast<nfs::SMSGProvidePassword *>(msg.get());
    if (rmsg == nullptr)
        return nfs::INVALID_SERVER_REQUEST;

    nfs::CMSGConnectInfoUsername cmsg(password.size(), password.c_str());
    if (nfs::send_message(m_sockfd, cmsg) < password.size())
        return nfs::TCP_ERROR;

    return nfs::OK;
}

ConnectReturn NFSConnection::access_filesystem(const std::string filesystemName) {
    std::unique_ptr<nfs::MSG> msg(nullptr);

    int result = nfs::wait_for_message(m_sockfd, msg);
    if (result <= 0 || msg == nullptr)
        return nfs::SERVER_NOT_RESPONDING;

    nfs::SMSGProvideFSName *rmsg = dynamic_cast<nfs::SMSGProvideFSName *>(msg.get());
    if (rmsg == nullptr)
        return nfs::INVALID_SERVER_REQUEST;

    nfs::CMSGConnectInfoUsername cmsg(filesystemName.size(), filesystemName.c_str());
    if (nfs::send_message(m_sockfd, cmsg) < filesystemName.size())
        return nfs::TCP_ERROR;

    result = nfs::wait_for_message(m_sockfd, msg);
    if (result <= 0 || msg == nullptr)
        return nfs::SERVER_NOT_RESPONDING;

    nfs::SMSGAuthorizationOk *armsg = dynamic_cast<nfs::SMSGAuthorizationOk *>(msg.get());
    if (rmsg == nullptr)
        return nfs::ACCESS_DENIED;

    return nfs::OK;
}

int NFSConnection::send_and_wait(nfs::MSG &clientMessage, std::unique_ptr<MSG> &resultMessage_ptr) {
    // Wysłanie wiadomości, sparwdzenie czy została wysłana
    int result = nfs::send_message(m_sockfd, clientMessage);
    if (result == 0) {
        m_errno = EHOSTUNREACH;
        return -1;
    }

    // Oczekiwanie na wiadomość zwrotną od serwera.
    // Obsługa błędów zwracanych przez funkcję.
    result = nfs::wait_for_message(m_sockfd, resultMessage_ptr);
    if (result == 0) {
        m_errno = EHOSTUNREACH;
        return -1;
    } else if (result < 0 || resultMessage_ptr == nullptr) {
        m_errno = EBADE;
        return -1;
    }

    // Poprawne wykonanie
    return 0;
}

} // namespace nfs
