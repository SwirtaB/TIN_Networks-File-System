#pragma once

#include "MessageBuffer.hpp"

#include <cassert>
#include <cstring>

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

    MSG(MSGCode code_) : code(static_cast<uint8_t>(code_)) {}

    virtual ~MSG() {}

    virtual void push_to_buffer(MessageBuffer &buffer) { buffer.push_uint8_t(code); }
};

struct CMSGConnectInfoUsername : MSG
{
  private:
    std::vector<char> _username;

  public:
    uint64_t username_size;
    char    *username;

    CMSGConnectInfoUsername(uint64_t username_size_, const char *username_) :
        MSG(MSGCode::CONNECT_INFO_USERNAME), _username(username_, username_ + username_size_),
        username_size(username_size_), username(_username.data()) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_char_data(username, username_size);
        buffer.push_uint64_t(username_size);
    }

    static CMSGConnectInfoUsername *from_buffer(MessageBuffer &buffer) {
        uint64_t          size = buffer.pop_uint64_t();
        std::vector<char> data = buffer.pop_char_data(size);
        return new CMSGConnectInfoUsername(size, data.data());
    }
};

struct CMSGConnectInfoPassword : MSG
{
  private:
    std::vector<char> _password;

  public:
    uint64_t password_size;
    char    *password;

    CMSGConnectInfoPassword(uint64_t password_size_, const char *password_) :
        MSG(MSGCode::CONNECT_INFO_PASSWORD), _password(password_, password_ + password_size_),
        password_size(password_size_), password(_password.data()) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_char_data(password, password_size);
        buffer.push_uint64_t(password_size);
    }

    static CMSGConnectInfoPassword *from_buffer(MessageBuffer &buffer) {
        uint64_t          size = buffer.pop_uint64_t();
        std::vector<char> data = buffer.pop_char_data(size);
        return new CMSGConnectInfoPassword(size, data.data());
    }
};

struct CMSGConnectInfoFSName : MSG
{
  private:
    std::vector<char> _fsname;

  public:
    uint64_t fsname_size;
    char    *fsname;

    CMSGConnectInfoFSName(uint64_t fsname_size_, const char *fsname_) :
        MSG(MSGCode::CONNECT_INFO_FSNAME), _fsname(fsname_, fsname_ + fsname_size_), fsname_size(fsname_size_),
        fsname(_fsname.data()) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_char_data(fsname, fsname_size);
        buffer.push_uint64_t(fsname_size);
    }

    static CMSGConnectInfoFSName *from_buffer(MessageBuffer &buffer) {
        uint64_t          size = buffer.pop_uint64_t();
        std::vector<char> data = buffer.pop_char_data(size);
        return new CMSGConnectInfoFSName(size, data.data());
    }
};

struct CMSGRequestOpen : MSG
{
  private:
    std::vector<char> _path;

  public:
    int64_t  oflag;
    int64_t  mode;
    uint64_t path_size;
    char    *path;

    CMSGRequestOpen(int64_t oflag_, int64_t mode_, uint64_t path_size_, const char *path_) :
        MSG(MSGCode::REQUEST_OPEN), _path(path_, path_ + path_size_), oflag(oflag_), mode(mode_), path_size(path_size_),
        path(_path.data()) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(oflag);
        buffer.push_int64_t(mode);
        buffer.push_char_data(path, path_size);
        buffer.push_uint64_t(path_size);
    }

    static CMSGRequestOpen *from_buffer(MessageBuffer &buffer) {
        uint64_t          size  = buffer.pop_uint64_t();
        std::vector<char> data  = buffer.pop_char_data(size);
        int64_t           mode  = buffer.pop_int64_t();
        int64_t           oflag = buffer.pop_int64_t();
        return new CMSGRequestOpen(oflag, mode, size, data.data());
    }
};

struct CMSGRequestClose : MSG
{
    int64_t fd;

    CMSGRequestClose(int64_t fd_) : MSG(MSGCode::REQUEST_CLOSE), fd(fd_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(fd);
    }

    static CMSGRequestClose *from_buffer(MessageBuffer &buffer) {
        int64_t fd = buffer.pop_int64_t();
        return new CMSGRequestClose(fd);
    }
};

struct CMSGRequestRead : MSG
{
    int64_t  fd;
    uint64_t size;

    CMSGRequestRead(int64_t fd_, uint64_t size_) : MSG(MSGCode::REQUEST_READ), fd(fd_), size(size_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(fd);
        buffer.push_uint64_t(size);
    }

    static CMSGRequestRead *from_buffer(MessageBuffer &buffer) {
        uint64_t size = buffer.pop_uint64_t();
        int64_t  fd   = buffer.pop_int64_t();
        return new CMSGRequestRead(fd, size);
    }
};

struct CMSGRequestWrite : MSG
{
  private:
    std::vector<char> _data;

  public:
    int64_t  fd;
    uint64_t data_size;
    char    *data;

    CMSGRequestWrite(int64_t fd_, uint64_t data_size_, const char *data_) :
        MSG(MSGCode::REQUEST_WRITE), _data(data_, data_ + data_size_), fd(fd_), data_size(data_size_),
        data(_data.data()) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(fd);
        buffer.push_char_data(data, data_size);
        buffer.push_uint64_t(data_size);
    }

    static CMSGRequestWrite *from_buffer(MessageBuffer &buffer) {
        uint64_t          size = buffer.pop_uint64_t();
        std::vector<char> data = buffer.pop_char_data(size);
        int64_t           fd   = buffer.pop_int64_t();
        return new CMSGRequestWrite(fd, size, data.data());
    }
};

struct CMSGRequestLseek : MSG
{
    int64_t fd;
    int64_t offset;
    int64_t whence;

    CMSGRequestLseek(int64_t fd_, int64_t offset_, int64_t whence_) :
        MSG(MSGCode::REQUEST_LSEEK), fd(fd_), offset(offset_), whence(whence_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(fd);
        buffer.push_int64_t(offset);
        buffer.push_int64_t(whence);
    }

    static CMSGRequestLseek *from_buffer(MessageBuffer &buffer) {
        int64_t whence = buffer.pop_int64_t();
        int64_t offset = buffer.pop_int64_t();
        int64_t fd     = buffer.pop_int64_t();
        return new CMSGRequestLseek(fd, offset, whence);
    }
};

struct CMSGRequestFstat : MSG
{
    int64_t fd;

    CMSGRequestFstat(int64_t fd_) : MSG(MSGCode::REQUEST_FSTAT), fd(fd_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(fd);
    }

    static CMSGRequestFstat *from_buffer(MessageBuffer &buffer) {
        int64_t fd = buffer.pop_int64_t();
        return new CMSGRequestFstat(fd);
    }
};

struct CMSGRequestUnlink : MSG
{
  private:
    std::vector<char> _path;

  public:
    uint64_t path_size;
    char    *path;

    CMSGRequestUnlink(uint64_t path_size_, const char *path_) :
        MSG(MSGCode::REQUEST_UNLINK), _path(path_, path_ + path_size_), path_size(path_size_), path(_path.data()) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_char_data(path, path_size);
        buffer.push_uint64_t(path_size);
    }

    static CMSGRequestUnlink *from_buffer(MessageBuffer &buffer) {
        uint64_t          size = buffer.pop_uint64_t();
        std::vector<char> data = buffer.pop_char_data(size);
        return new CMSGRequestUnlink(size, data.data());
    }
};

struct CMSGRequestFlock : MSG
{
    int64_t fd;
    int64_t operation;

    CMSGRequestFlock(int64_t fd_, int64_t operation_) : MSG(MSGCode::REQUEST_FLOCK), fd(fd_), operation(operation_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(fd);
        buffer.push_int64_t(operation);
    }

    static CMSGRequestFlock *from_buffer(MessageBuffer &buffer) {
        int64_t operation = buffer.pop_int64_t();
        int64_t fd        = buffer.pop_int64_t();
        return new CMSGRequestFlock(fd, operation);
    }
};

struct CMSGDisconnect : MSG
{
    CMSGDisconnect() : MSG(MSGCode::DISCONNECT) {}
};

struct SMSGProvideUsername : MSG
{
    SMSGProvideUsername() : MSG(MSGCode::PROVIDE_USERNAME) {}
};

struct SMSGProvidePassword : MSG
{
    SMSGProvidePassword() : MSG(MSGCode::PROVIDE_PASSWORD) {}
};

struct SMSGProvideFSName : MSG
{
    SMSGProvideFSName() : MSG(MSGCode::PROVIDE_FSNAME) {}
};

struct SMSGAuthorizationOk : MSG
{
    SMSGAuthorizationOk() : MSG(MSGCode::AUTHORIZATION_OK) {}
};

struct SMSGAuthorizationFailed : MSG
{
    SMSGAuthorizationFailed() : MSG(MSGCode::AUTHORIZATION_FAILED) {}
};

struct SMSGResultOpen : MSG
{
    int64_t fd;
    int64_t _errno;

    SMSGResultOpen(int64_t fd_, int64_t errno_) : MSG(MSGCode::RESULT_OPEN), fd(fd_), _errno(errno_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(fd);
        buffer.push_int64_t(_errno);
    }

    static SMSGResultOpen *from_buffer(MessageBuffer &buffer) {
        int64_t _errno = buffer.pop_int64_t();
        int64_t fd     = buffer.pop_int64_t();
        return new SMSGResultOpen(fd, _errno);
    }
};

struct SMSGResultClose : MSG
{
    int64_t result;
    int64_t _errno;

    SMSGResultClose(int64_t result_, int64_t errno_) : MSG(MSGCode::RESULT_CLOSE), result(result_), _errno(errno_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(result);
        buffer.push_int64_t(_errno);
    }

    static SMSGResultClose *from_buffer(MessageBuffer &buffer) {
        int64_t _errno = buffer.pop_int64_t();
        int64_t fd     = buffer.pop_int64_t();
        return new SMSGResultClose(fd, _errno);
    }
};

struct SMSGResultRead : MSG
{
  private:
    std::vector<char> _data;

  public:
    int64_t  _errno;
    int64_t data_size;
    char    *data;

    SMSGResultRead(int64_t errno_, int64_t data_size_, const char *data_) :
        MSG(MSGCode::RESULT_READ), _data(data_, data_ + (data_size_ >= 0 ? data_size_ : 0)), _errno(errno_), data_size(data_size_),
        data(_data.data()) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(_errno);
        buffer.push_char_data(data, data_size);
        buffer.push_int64_t(data_size);
    }

    static SMSGResultRead *from_buffer(MessageBuffer &buffer) {
        int64_t           size   = buffer.pop_int64_t();
        std::vector<char> data   = buffer.pop_char_data(size);
        int64_t           _errno = buffer.pop_int64_t();
        return new SMSGResultRead(_errno, size, data.data());
    }
};

struct SMSGResultWrite : MSG
{
    int64_t result;
    int64_t _errno;

    SMSGResultWrite(int64_t result_, int64_t errno_) : MSG(MSGCode::RESULT_WRITE), result(result_), _errno(errno_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(result);
        buffer.push_int64_t(_errno);
    }

    static SMSGResultWrite *from_buffer(MessageBuffer &buffer) {
        int64_t _errno = buffer.pop_int64_t();
        int64_t result = buffer.pop_int64_t();
        return new SMSGResultWrite(result, _errno);
    }
};

struct SMSGResultLseek : MSG
{
    int64_t offset;
    int64_t _errno;

    SMSGResultLseek(int64_t offset_, int64_t errno_) : MSG(MSGCode::RESULT_LSEEK), offset(offset_), _errno(errno_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(offset);
        buffer.push_int64_t(_errno);
    }

    static SMSGResultLseek *from_buffer(MessageBuffer &buffer) {
        int64_t _errno = buffer.pop_int64_t();
        int64_t offset = buffer.pop_int64_t();
        return new SMSGResultLseek(offset, _errno);
    }
};

struct SMSGResultFstat : MSG
{
    int64_t     result;
    int64_t     _errno;
    struct stat statbuf;

    SMSGResultFstat(int64_t result_, int64_t errno_, struct stat statbuf_) :
        MSG(MSGCode::RESULT_FSTAT), result(result_), _errno(errno_) {
        statbuf = statbuf_;
    }

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(result);
        buffer.push_int64_t(_errno);
        buffer.push_stat(statbuf);
    }

    static SMSGResultFstat *from_buffer(MessageBuffer &buffer) {
        struct stat statbuf = buffer.pop_stat();
        int64_t     _errno  = buffer.pop_int64_t();
        int64_t     result  = buffer.pop_int64_t();
        return new SMSGResultFstat(result, _errno, statbuf);
    }
};

struct SMSGResultUnlink : MSG
{
    int64_t result;
    int64_t _errno;

    SMSGResultUnlink(int64_t result_, int64_t errno_) : MSG(MSGCode::RESULT_UNLINK), result(result_), _errno(errno_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(result);
        buffer.push_int64_t(_errno);
    }

    static SMSGResultUnlink *from_buffer(MessageBuffer &buffer) {
        int64_t _errno = buffer.pop_int64_t();
        int64_t result = buffer.pop_int64_t();
        return new SMSGResultUnlink(result, _errno);
    }
};

struct SMSGResultFlock : MSG
{
    int64_t result;
    int64_t _errno;

    SMSGResultFlock(int64_t result_, int64_t errno_) : MSG(MSGCode::RESULT_FLOCK), result(result_), _errno(errno_) {}

    void push_to_buffer(MessageBuffer &buffer) override {
        MSG::push_to_buffer(buffer);
        buffer.push_int64_t(result);
        buffer.push_int64_t(_errno);
    }

    static SMSGResultFlock *from_buffer(MessageBuffer &buffer) {
        int64_t _errno = buffer.pop_int64_t();
        int64_t result = buffer.pop_int64_t();
        return new SMSGResultFlock(result, _errno);
    }
};

struct MSGUnexpectedError : MSG
{
    MSGUnexpectedError() : MSG(MSGCode::UNEXPECTED_ERROR) {}
};
} // namespace nfs
