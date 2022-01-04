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
        uint8_t code;

        MSG(MSGCode code_)
            : code(static_cast<uint8_t>(code_))
        {
        }

        virtual ~MSG() {}
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
        uint64_t username_size;
        char *username;

        CMSGConnectInfoUsername(uint64_t username_size_, char *username_)
            : MSG(MSGCode::CONNECT_INFO_USERNAME), _username(username_, username_ + username_size_), username_size(username_size_), username(_username.data())
        {
        }

    private:
        std::vector<char> _username;
    };

    struct CMSGConnectInfoPassword : MSG
    {
        uint64_t password_size;
        char *password;

        CMSGConnectInfoPassword(uint64_t password_size_, char *password_)
            : MSG(MSGCode::CONNECT_INFO_PASSWORD), _password(password_, password_ + password_size_), password_size(password_size_), password(_password.data())
        {
        }

    private:
        std::vector<char> _password;
    };

    struct CMSGConnectInfoFSName : MSG
    {
        uint64_t fsname_size;
        char *fsname;

        CMSGConnectInfoFSName(uint64_t fsname_size_, char *fsname_)
            : MSG(MSGCode::CONNECT_INFO_FSNAME), _fsname(fsname_, fsname_ + fsname_size_), fsname_size(fsname_size_), fsname(_fsname.data())
        {
        }

    private:
        std::vector<char> _fsname;
    };

    struct CMSGRequestOpen : MSG
    {
        uint64_t oflag;
        uint64_t mode;
        uint64_t path_size;
        char *path;

        CMSGRequestOpen(uint64_t oflag_, uint64_t mode_, uint64_t path_size_, char *path_)
            : MSG(MSGCode::REQUEST_OPEN), oflag(oflag_), mode(mode_), _path(path_, path_ + path_size_), path_size(path_size_), path(_path.data())
        {
        }

    private:
        std::vector<char> _path;
    };

    struct CMSGRequestClose : MSG
    {
        int64_t fd;

        CMSGRequestClose(int64_t fd_)
            : MSG(MSGCode::REQUEST_CLOSE), fd(fd_)
        {
        }
    };

    struct CMSGRequestRead : MSG
    {
        int64_t fd;
        uint64_t size;

        CMSGRequestRead(int64_t fd_, uint64_t size_)
            : MSG(MSGCode::REQUEST_READ), fd(fd_), size(size_)
        {
        }
    };

    struct CMSGRequestWrite : MSG
    {
        int64_t fd;
        uint64_t data_size;
        char *data;

        CMSGRequestWrite(int64_t fd_, uint64_t data_size_, char *data_)
            : MSG(MSGCode::REQUEST_WRITE), fd(fd_), _data(data_, data_ + data_size_), data_size(data_size_), data(_data.data())
        {
        }

    private:
        std::vector<char> _data;
    };

    struct CMSGRequestLseek : MSG
    {
        int64_t fd;
        int64_t offset;
        int64_t whence;

        CMSGRequestLseek(int64_t fd_, int64_t offset_, int64_t whence_)
            : MSG(MSGCode::REQUEST_LSEEK), fd(fd_), offset(offset_), whence(whence_)
        {
        }
    };

    struct CMSGRequestFstat : MSG
    {
        int64_t fd;

        CMSGRequestFstat(int64_t fd_)
            : MSG(MSGCode::REQUEST_FSTAT), fd(fd_)
        {
        }
    };

    struct CMSGRequestUnlink : MSG
    {
        uint64_t path_size;
        char *path;

        CMSGRequestUnlink(uint64_t path_size_, char *path_)
            : MSG(MSGCode::REQUEST_UNLINK), _path(path_, path_ + path_size_), path_size(path_size_), path(_path.data())
        {
        }

    private:
        std::vector<char> _path;
    };

    struct CMSGRequestFlock : MSG
    {
        int64_t fd;
        int64_t operation;

        CMSGRequestFlock(int64_t fd_, int64_t operation_)
            : MSG(MSGCode::REQUEST_FLOCK), fd(fd_), operation(operation_)
        {
        }
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
        int64_t fd;
        int64_t _errno;

        SMSGResultOpen(int64_t fd_, int64_t errno_)
            : MSG(MSGCode::RESULT_OPEN), fd(fd_), _errno(errno_)
        {
        }
    };

    struct SMSGResultClose : MSG
    {
        int64_t result;
        int64_t _errno;

        SMSGResultClose(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_CLOSE), result(result_), _errno(errno_)
        {
        }
    };

    struct SMSGResultRead : MSG
    {
        int64_t _errno;
        uint64_t data_size;
        char *data;

        SMSGResultRead(int64_t errno_, uint64_t data_size_, char *data_)
            : MSG(MSGCode::RESULT_READ), _errno(errno_), _data(data_, data_ + data_size_), data_size(data_size_), data(_data.data())
        {
        }

    private:
        std::vector<char> _data;
    };

    struct SMSGResultWrite : MSG
    {
        int64_t result;
        int64_t _errno;

        SMSGResultWrite(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_WRITE), result(result_), _errno(errno_)
        {
        }
    };

    struct SMSGResultLseek : MSG
    {
        int64_t offset;
        int64_t _errno;

        SMSGResultLseek(int64_t offset_, int64_t errno_)
            : MSG(MSGCode::RESULT_LSEEK), offset(offset_), _errno(errno_)
        {
        }
    };

    struct SMSGResultFstat : MSG
    {
        int64_t result;
        int64_t _errno;
        struct stat statbuf;

        SMSGResultFstat(int64_t result_, int64_t errno_, struct stat statbuf_)
            : MSG(MSGCode::RESULT_FSTAT), result(result_), _errno(errno_)
        {
            statbuf = statbuf_;
        }
    };

    struct SMSGResultUnlink : MSG
    {
        int64_t result;
        int64_t _errno;

        SMSGResultUnlink(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_UNLINK), result(result_), _errno(errno_)
        {
        }
    };

    struct SMSGResultFlock : MSG
    {
        int64_t result;
        int64_t _errno;

        SMSGResultFlock(int64_t result_, int64_t errno_)
            : MSG(MSGCode::RESULT_FLOCK), result(result_), _errno(errno_)
        {
        }
    };

    struct MSGUnexpectedError : MSG
    {
        MSGUnexpectedError()
            : MSG(MSGCode::UNEXPECTED_ERROR)
        {
        }
    };
}
