#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>
#include <vector>

extern "C"
{
#include <sys/stat.h>
}

namespace nfs
{

    enum class MSGCode : uint8_t
    {
        // client
        CONNECT_START,
        CONNECT_INFO_USERNAME,
        CONNECT_INFO_PASSWORD,
        CONNECT_INFO_FSNAME,
        REQUEST_OPEN,
        REQUEST_CLOSE,
        REQUEST_READ,
        REQUEST_WRITE,
        REQUEST_LSEEK,
        REQUEST_FSTAT,
        REQUEST_UNLINK,
        REQUEST_FLOCK,
        DISCONNECT,
        // server
        PROVIDE_USERNAME,
        PROVIDE_PASSWORD,
        PROVIDE_FSNAME,
        AUTHORIZATION_OK,
        AUTHORIZATION_FAILED,
        RESULT_OPEN,
        RESULT_CLOSE,
        RESULT_READ,
        RESULT_WRITE,
        RESULT_LSEEK,
        RESULT_FSTAT,
        RESULT_UNLINK,
        RESULT_FLOCK,
        // common
        UNEXPECTED_ERROR,
    };

    struct MSG
    {
        MSG(MSGCode code_)
            : code(static_cast<uint8_t>(code_))
        {
        }

        virtual ~MSG() {}

        uint8_t code;
    };

    struct CMSGConnectStart : MSG
    {
        CMSGConnectStart()
            : MSG(MSGCode::CONNECT_START)
        {
        }
    };

    struct CMSGConnectInfoUsername : MSG
    {
        CMSGConnectInfoUsername(uint64_t username_size_, char *username_)
            : MSG(MSGCode::CONNECT_INFO_USERNAME), username(username_, username_ + username_size_)
        {
        }

        std::vector<char> username;
    };

    struct CMSGConnectInfoPassword : MSG
    {
        CMSGConnectInfoPassword(uint64_t password_size_, char *password_)
            : MSG(MSGCode::CONNECT_INFO_PASSWORD), password(password_, password_ + password_size_)
        {
        }

        std::vector<char> password;
    };

    struct CMSGConnectInfoFSName : MSG
    {
        CMSGConnectInfoFSName(uint64_t fsname_size_, char *fsname_)
            : MSG(MSGCode::CONNECT_INFO_FSNAME), fsname(fsname_, fsname_ + fsname_size_)
        {
        }

        std::vector<char> fsname;
    };

    struct CMSGRequestOpen : MSG
    {
        CMSGRequestOpen(uint64_t oflag_, uint64_t mode_, uint64_t path_size_, char *path_)
            : MSG(MSGCode::REQUEST_OPEN), oflag(oflag_), mode(mode_), path(path_, path_ + path_size_)
        {
        }

        uint64_t oflag;
        uint64_t mode;
        std::vector<char> path;
    };

    struct CMSGRequestClose : MSG
    {
        CMSGRequestClose(int64_t fd_)
            : MSG(MSGCode::REQUEST_CLOSE), fd(fd_)
        {
        }

        int64_t fd;
    };

    struct CMSGRequestRead : MSG
    {
        CMSGRequestRead(int64_t fd_, uint64_t size_)
            : MSG(MSGCode::REQUEST_READ), fd(fd_), size(size_)
        {
        }

        int64_t fd;
        uint64_t size;
    };

    struct CMSGRequestWrite : MSG
    {
        CMSGRequestWrite(int64_t fd_, uint64_t size_, char *data_)
            : MSG(MSGCode::REQUEST_WRITE), fd(fd_), data(data_, data_ + size_)
        {
        }

        int64_t fd;
        std::vector<char> data;
    };

    struct CMSGRequestLseek : MSG
    {
        CMSGRequestLseek(int64_t fd_, int64_t offset_, int64_t whence_)
            : MSG(MSGCode::REQUEST_LSEEK), fd(fd_), offset(offset_), whence(whence_)
        {
        }

        int64_t fd;
        int64_t offset;
        int64_t whence;
    };

    struct CMSGRequestFstat : MSG
    {
        CMSGRequestFstat(int64_t fd_)
            : MSG(MSGCode::REQUEST_FSTAT), fd(fd_)
        {
        }

        int64_t fd;
    };

    struct CMSGRequestUnlink : MSG
    {
        CMSGRequestUnlink(uint64_t path_size_, char *path_)
            : MSG(MSGCode::REQUEST_UNLINK), path(path_, path_ + path_size_)
        {
        }

        std::vector<char> path;
    };

    struct CMSGRequestFlock : MSG
    {
        CMSGRequestFlock(int64_t fd_, int64_t operation_)
            : MSG(MSGCode::REQUEST_FLOCK), fd(fd_), operation(operation_)
        {
        }

        int64_t fd;
        int64_t operation;
    };

    struct CMSGDisconnect : MSG
    {
        CMSGDisconnect()
            : MSG(MSGCode::DISCONNECT)
        {
        }
    };

    struct SMSGProvideUsername : MSG
    {
        SMSGProvideUsername()
            : MSG(MSGCode::PROVIDE_USERNAME)
        {
        }
    };

    struct SMSGProvidePassword : MSG
    {
        SMSGProvidePassword()
            : MSG(MSGCode::PROVIDE_PASSWORD)
        {
        }
    };

    struct SMSGProvideFSName : MSG
    {
        SMSGProvideFSName()
            : MSG(MSGCode::PROVIDE_FSNAME)
        {
        }
    };

    struct SMSGAuthorizationOk : MSG
    {
        SMSGAuthorizationOk()
            : MSG(MSGCode::AUTHORIZATION_OK)
        {
        }
    };

    struct SMSGAuthorizationFailed : MSG
    {
        SMSGAuthorizationFailed()
            : MSG(MSGCode::AUTHORIZATION_FAILED)
        {
        }
    };

    struct SMSGResultOpen : MSG
    {
        SMSGResultOpen(int64_t fd_, int64_t errno_)
            : MSG(MSGCode::RESULT_OPEN), fd(fd_), _errno(errno_)
        {
        }

        int64_t fd;
        int64_t _errno;
    };

    struct SMSGResultClose : MSG
    {
        SMSGResultClose(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_CLOSE), result(result_), _errno(errno_)
        {
        }

        int64_t result;
        int64_t _errno;
    };

    struct SMSGResultRead : MSG
    {
        SMSGResultRead(uint64_t size_, int64_t errno_, char *data_)
            : MSG(MSGCode::RESULT_READ), _errno(errno_), data(data_, data_ + size_)
        {
        }

        int64_t _errno;
        std::vector<char> data;
    };

    struct SMSGResultWrite : MSG
    {
        SMSGResultWrite(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_WRITE), result(result_), _errno(errno_)
        {
        }

        int64_t result;
        int64_t _errno;
    };

    struct SMSGResultLseek : MSG
    {
        SMSGResultLseek(int64_t offset_, int64_t errno_)
            : MSG(MSGCode::RESULT_LSEEK), offset(offset_), _errno(errno_)
        {
        }

        int64_t offset;
        int64_t _errno;
    };

    struct SMSGResultFstat : MSG
    {
        SMSGResultFstat(int64_t result_, int64_t errno_, struct stat statbuf_)
            : MSG(MSGCode::RESULT_FSTAT), result(result_), _errno(errno_)
        {
            statbuf = statbuf_;
        }

        int64_t result;
        int64_t _errno;
        struct stat statbuf;
    };

    struct SMSGResultUnlink : MSG
    {
        SMSGResultUnlink(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_UNLINK), result(result_), _errno(errno_)
        {
        }

        int64_t result;
        int64_t _errno;
    };

    struct SMSGResultFlock : MSG
    {
        SMSGResultFlock(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_FLOCK), result(result_), _errno(errno_)
        {
        }

        int64_t result;
        int64_t _errno;
    };

    struct MSGUnexpectedError : MSG
    {
        MSGUnexpectedError()
            : MSG(MSGCode::UNEXPECTED_ERROR)
        {
        }
    };
}
