#include "NFSConnection.hpp"

namespace nfs
{

NFSConnection::NFSConnection() : m_access(false), m_sockfd(-1) {}

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
} // namespace nfs
