#pragma once

#include "NFSServer.hpp"

#include <string>

namespace nfs
{

class NFSServerWorker
{
  public:
    NFSServerWorker(NFSServerConfig config_, int client_socket_);
    ~NFSServerWorker();

    int run();

  private:
    NFSServerConfig config;
    int             client_socket;
    bool            authenticated = false;
    int             userid;
    std::string     username;
    std::string     fsname;
    std::string     filesystem_prefix;

    std::unordered_map<int, int> descriptor_map;
    int                          next_descriptor = 1;

    int authenitcate_user();
    int request_username(std::unique_ptr<CMSGConnectInfoUsername> &msg);
    int request_password(std::unique_ptr<CMSGConnectInfoPassword> &msg);
    int request_fsname(std::unique_ptr<CMSGConnectInfoFSName> &msg);

    int handle_requests();
    int handle_request_open(CMSGRequestOpen &msg);
    int handle_request_close(CMSGRequestClose &msg);
    int handle_request_read(CMSGRequestRead &msg);
    int handle_request_write(CMSGRequestWrite &msg);
    int handle_request_lseek(CMSGRequestLseek &msg);
    int handle_request_fstat(CMSGRequestFstat &msg);
    int handle_request_unlink(CMSGRequestUnlink &msg);
    int handle_request_flock(CMSGRequestFlock &msg);

    int  add_descriptor_to_map(int file_descriptor);
    bool is_descriptor_in_map(int client_descriptor);
    int  get_descriptor_from_map(int client_descriptor);
    void remove_descriptor_from_map(int client_descriptor);

    bool        select_user(char *username, char *password);
    bool        select_filesystem(char *filesystem);
    std::string get_path_in_filesystem(char *path);

    int enter_user_mode(int userid);
    int exit_user_mode();
};
} // namespace nfs