#include "../include/NFSServerWorker.hpp"

#include "../include/Logging.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <optional>

extern "C"
{
#include <fcntl.h>
#include <pwd.h>
#include <sys/file.h>
#include <unistd.h>

#ifdef __unix__
#include <crypt.h>
#include <shadow.h>
#endif
}

namespace nfs
{

NFSServerWorker::NFSServerWorker(NFSServerConfig config_, int client_socket_) :
    config(config_), client_socket(client_socket_) {}

NFSServerWorker::~NFSServerWorker() {
    for (auto mapping = descriptor_map.begin(); mapping != descriptor_map.end(); ++mapping) {
        int res = close(mapping->second);
        log_cerr("Closed leftover fd ", mapping->second, " with result ", res);
    }
}

int NFSServerWorker::run() {
    int auth = authenitcate_user();

    if (auth != 0) {
        log_cerr("Authentication failed");
        return auth;
    }

    if (!authenticated)
        return 0;

    return handle_requests();
}

int NFSServerWorker::authenitcate_user() {

    std::unique_ptr<CMSGConnectInfoUsername> username_info;

    int res_username = request_username(username_info);
    if (res_username != 0) {
        log_cerr("Requesting username failed");
        return res_username;
    }

    std::unique_ptr<CMSGConnectInfoPassword> password_info;

    int res_password = request_password(password_info);
    if (res_password != 0) {
        log_cerr("Requesting password failed");
        return res_password;
    }

    std::unique_ptr<CMSGConnectInfoFSName> fsname_info;

    int res_fsname = request_fsname(fsname_info);
    if (res_fsname != 0) {
        log_cerr("Requesting fsname failed");
        return res_fsname;
    }

    bool res_user = select_user(username_info->username, password_info->password);
    if (!res_user) {
        log_cerr("Failed to select user ", username_info->username);
    }
    bool res_fs = select_filesystem(fsname_info->fsname);
    if (!res_fs) {
        log_cerr("Failed to select filesystem ", fsname_info->fsname);
    }

    authenticated = res_user && res_fs;

    std::unique_ptr<MSG> auth_msg;
    if (authenticated) {
        auth_msg.reset(new SMSGAuthorizationOk);
        username = username_info->username;
        fsname   = fsname_info->fsname;
        log_cerr("Authenticated ", username, " in ", fsname);
    } else {
        auth_msg.reset(new SMSGAuthorizationFailed);
        log_cerr("Authentication for ", username_info->username, " failed");
    }
    int auth_msg_res = send_message(client_socket, *auth_msg);
    if (auth_msg_res <= 0) {
        log_perror("Failed to send auth msg");
        return -1;
    }

    return 0;
}

int NFSServerWorker::request_username(std::unique_ptr<CMSGConnectInfoUsername> &msg) {
    SMSGProvideUsername req_msg;
    int                 res = send_message(client_socket, req_msg);
    if (res <= 0) {
        log_perror("Failed to send username request");
        return -1;
    }

    std::unique_ptr<MSG> response;
    int                  res_res = wait_for_message(client_socket, response);
    if (res_res <= 0) {
        log_perror("Failed to receive username");
        return -1;
    }

    CMSGConnectInfoUsername *username = dynamic_cast<CMSGConnectInfoUsername *>(response.get());
    if (username == nullptr) {
        log_cerr("Username message invalid");
        return EBADMSG;
    } else if (username->username[username->username_size - 1] != 0) {
        log_cerr("Username string invalid");
        return EBADMSG;
    }

    (void)response.release();
    msg.reset(username);
    return 0;
}

int NFSServerWorker::request_password(std::unique_ptr<CMSGConnectInfoPassword> &msg) {
    SMSGProvidePassword req_msg;
    int                 res = send_message(client_socket, req_msg);
    if (res <= 0) {
        log_perror("Failed to send password request");
        return -1;
    }

    std::unique_ptr<MSG> response;
    int                  res_res = wait_for_message(client_socket, response);
    if (res_res <= 0) {
        log_perror("Failed to receive password");
        return -1;
    }

    CMSGConnectInfoPassword *password = dynamic_cast<CMSGConnectInfoPassword *>(response.get());
    if (password == nullptr) {
        log_cerr("Password message invalid");
        return EBADMSG;
    } else if (password->password[password->password_size - 1] != 0) {
        log_cerr("Password string invalid");
        return EBADMSG;
    }

    (void)response.release();
    msg.reset(password);
    return 0;
}

int NFSServerWorker::request_fsname(std::unique_ptr<CMSGConnectInfoFSName> &msg) {
    SMSGProvideFSName req_msg;
    int               res = send_message(client_socket, req_msg);
    if (res <= 0) {
        log_perror("Failed to send FSName request");
        return -1;
    }

    std::unique_ptr<MSG> response;
    int                  res_res = wait_for_message(client_socket, response);
    if (res_res <= 0) {
        log_perror("Failed to receive FSName");
        return -1;
    }

    CMSGConnectInfoFSName *fsname = dynamic_cast<CMSGConnectInfoFSName *>(response.get());
    if (fsname == nullptr) {
        log_cerr("FSName message invalid");
        return EBADMSG;
    } else if (fsname->fsname[fsname->fsname_size - 1] != 0) {
        log_cerr("FSName string invalid");
        return EBADMSG;
    }

    (void)response.release();
    msg.reset(fsname);
    return 0;
}

int NFSServerWorker::handle_requests() {
    while (true) {
        std::unique_ptr<MSG> request;
        int                  request_res = wait_for_message(client_socket, request);
        if (request_res <= 0) {
            log_perror("Failed to receive request");
            return -1;
        }

        log_cerr("Received request with code ", static_cast<int>(request->code));

        if (enter_user_mode(userid) != 0) {
            log_perror("Failed to enter user mode");
            return -1;
        };

        std::optional<int> result;

        CMSGRequestOpen *request_open = dynamic_cast<CMSGRequestOpen *>(request.get());
        if (request_open != nullptr)
            result = handle_request_open(*request_open);
        CMSGRequestClose *request_close = dynamic_cast<CMSGRequestClose *>(request.get());
        if (request_close != nullptr)
            result = handle_request_close(*request_close);
        CMSGRequestRead *request_read = dynamic_cast<CMSGRequestRead *>(request.get());
        if (request_read != nullptr)
            result = handle_request_read(*request_read);
        CMSGRequestWrite *request_write = dynamic_cast<CMSGRequestWrite *>(request.get());
        if (request_write != nullptr)
            result = handle_request_write(*request_write);
        CMSGRequestLseek *request_lseek = dynamic_cast<CMSGRequestLseek *>(request.get());
        if (request_lseek != nullptr)
            result = handle_request_lseek(*request_lseek);
        CMSGRequestFstat *request_fstat = dynamic_cast<CMSGRequestFstat *>(request.get());
        if (request_fstat != nullptr)
            result = handle_request_fstat(*request_fstat);
        CMSGRequestUnlink *request_unlink = dynamic_cast<CMSGRequestUnlink *>(request.get());
        if (request_unlink != nullptr)
            result = handle_request_unlink(*request_unlink);
        CMSGRequestFlock *request_flock = dynamic_cast<CMSGRequestFlock *>(request.get());
        if (request_flock != nullptr)
            result = handle_request_flock(*request_flock);
        CMSGDisconnect *disconnect = dynamic_cast<CMSGDisconnect *>(request.get());
        if (disconnect != nullptr) {
            exit_user_mode();
            log_cerr("Server worker disconnecting ", username, " in ", fsname);
            return 0;
        }

        if (exit_user_mode() != 0) {
            log_perror("Failed to exit user mode");
            return -1;
        };

        if (!result.has_value()) {
            log_cerr("Received invalid request");
            return EBADMSG;
        } else if (result.value() != 0) {
            log_perror("Failed to handle request");
            return result.value();
        }
    }
}

int NFSServerWorker::handle_request_open(CMSGRequestOpen &msg) {
    if (msg.path[msg.path_size - 1] != 0) {
        log_cerr("Open path string invaid");
        return -1;
    }

    log_cerr("open for ", username, " in ", fsname, " at ", msg.path);

    std::string    path = get_path_in_filesystem(msg.path);
    int            fd   = open(path.c_str(), msg.oflag, msg.mode);
    SMSGResultOpen response(fd > 0 ? add_descriptor_to_map(fd) : fd, fd > 0 ? 0 : errno);
    int            res = send_message(client_socket, response);
    if (res <= 0) {
        log_perror("Failed to send open result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::handle_request_close(CMSGRequestClose &msg) {
    log_cerr("close for ", username, " in ", fsname, " for cd ", msg.fd);

    int res;
    int errno_ = 0;

    if (is_descriptor_in_map(msg.fd)) {
        res = close(get_descriptor_from_map(msg.fd));
        remove_descriptor_from_map(msg.fd);
        if (res < 0) {
            errno_ = errno;
        }
    } else {
        res    = -1;
        errno_ = EBADF;
    }

    SMSGResultClose response(res, errno_);
    int             send_res = send_message(client_socket, response);
    if (send_res <= 0) {
        log_perror("Failed to send close result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::handle_request_read(CMSGRequestRead &msg) {
    log_cerr("read for ", username, " in ", fsname, " for cd ", msg.fd);

    int   res;
    int   errno_ = 0;
    char *buff   = new char[msg.size];

    if (is_descriptor_in_map(msg.fd)) {
        res = read(get_descriptor_from_map(msg.fd), buff, msg.size);
        if (res < 0) {
            errno_ = errno;
        }
    } else {
        res    = -1;
        errno_ = EBADF;
    }

    SMSGResultRead response(errno_, res, buff);
    delete[] buff;
    int send_res = send_message(client_socket, response);
    if (send_res <= 0) {
        log_perror("Failed to send read result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::handle_request_write(CMSGRequestWrite &msg) {
    log_cerr("write for ", username, " in ", fsname, " for cd ", msg.fd);

    int res;
    int errno_ = 0;

    if (is_descriptor_in_map(msg.fd)) {
        res = write(get_descriptor_from_map(msg.fd), msg.data, msg.data_size);
        if (res < 0) {
            errno_ = errno;
        }
    } else {
        res    = -1;
        errno_ = EBADF;
    }

    SMSGResultWrite response(res, errno_);
    int             send_res = send_message(client_socket, response);
    if (send_res <= 0) {
        log_perror("Failed to send write result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::handle_request_lseek(CMSGRequestLseek &msg) {
    log_cerr("lseek for ", username, " in ", fsname, " for cd ", msg.fd);

    int res;
    int errno_ = 0;

    if (is_descriptor_in_map(msg.fd)) {
        res = lseek(get_descriptor_from_map(msg.fd), msg.offset, msg.whence);
        if (res != 0) {
            errno_ = errno;
        }
    } else {
        res    = -1;
        errno_ = EBADF;
    }

    SMSGResultLseek response(res, errno_);
    int             send_res = send_message(client_socket, response);
    if (send_res <= 0) {
        log_perror("Failed to send lseek result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::handle_request_fstat(CMSGRequestFstat &msg) {
    log_cerr("fstat for ", username, " in ", fsname, " for cd ", msg.fd);

    int         res;
    struct stat statbuf;
    int         errno_ = 0;

    if (is_descriptor_in_map(msg.fd)) {
        res = fstat(get_descriptor_from_map(msg.fd), &statbuf);
        if (res != 0) {
            errno_ = errno;
        }
    } else {
        res    = -1;
        errno_ = EBADF;
    }

    SMSGResultFstat response(res, errno_, statbuf);
    int             send_res = send_message(client_socket, response);
    if (send_res <= 0) {
        log_perror("Failed to send fstat result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::handle_request_unlink(CMSGRequestUnlink &msg) {
    if (msg.path[msg.path_size - 1] != 0) {
        log_cerr("Unlink path string invaid");
        return -1;
    }

    log_cerr("unlink for ", username, " in ", fsname, " at ", msg.path);

    std::string    path = get_path_in_filesystem(msg.path);
    int            res  = unlink(path.c_str());
    SMSGResultUnlink response(res, res == 0 ? 0 : errno);
    int            send_res = send_message(client_socket, response);
    if (send_res <= 0) {
        log_perror("Failed to send unlink result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::handle_request_flock(CMSGRequestFlock &msg) {
    log_cerr("fstat for ", username, " in ", fsname, " for cd ", msg.fd);

    int res;
    int errno_ = 0;

    if (is_descriptor_in_map(msg.fd)) {
        res = flock(get_descriptor_from_map(msg.fd), msg.operation);
        if (res != 0) {
            errno_ = errno;
        }
    } else {
        res    = -1;
        errno_ = EBADF;
    }

    SMSGResultFlock response(res, errno_);
    int             send_res = send_message(client_socket, response);
    if (send_res <= 0) {
        log_perror("Failed to send flock result");
        return -1;
    }

    return 0;
}

int NFSServerWorker::add_descriptor_to_map(int file_descriptor) {
    int client_descriptor             = next_descriptor++;
    descriptor_map[client_descriptor] = file_descriptor;
    log_cerr("generated cd ", client_descriptor, " for fd ", file_descriptor);
    return client_descriptor;
}

bool NFSServerWorker::is_descriptor_in_map(int client_descriptor) {
    return descriptor_map.count(client_descriptor) > 0;
}

int NFSServerWorker::get_descriptor_from_map(int client_descriptor) {
    return descriptor_map[client_descriptor];
}

void NFSServerWorker::remove_descriptor_from_map(int client_descriptor) {
    descriptor_map.erase(client_descriptor);
}

bool NFSServerWorker::select_user(char *username, char *password) {
    struct passwd *user_info;
    user_info = getpwnam(username);
#ifdef __unix__
    if (user_info == NULL)
        return false;
    if (strcmp(user_info->pw_passwd, "x") != 0) {
        if (strcmp(user_info->pw_passwd, crypt(password, user_info->pw_passwd)) != 0)
            return false;
    } else {
        struct spwd *shadow_entry = getspnam(username);
        if (shadow_entry == NULL)
            return false;
        if (strcmp(shadow_entry->sp_pwdp, crypt(password, shadow_entry->sp_pwdp)) != 0)
            return false;
    }

    userid = user_info->pw_uid;
    return true;
#endif
#ifndef __unix__
    log_cerr("User authentication only supported on __unix__");
    return false;
#endif
}

bool NFSServerWorker::select_filesystem(char *filesystem) {
    if (config.filesystems.count(filesystem) == 0)
        return false;
    filesystem_prefix = config.filesystems[filesystem];
    return true;
}

std::string NFSServerWorker::get_path_in_filesystem(char *path) {
    return filesystem_prefix + std::string("/") + std::string(path);
}

int NFSServerWorker::enter_user_mode(int userid) {
    return seteuid(userid);
}

int NFSServerWorker::exit_user_mode() {
    return seteuid(getuid());
}

} // namespace nfs