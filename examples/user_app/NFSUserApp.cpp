#include "NFSUserApp.hpp"
#include <fstream>
#include <iostream>
#include <iterator>

extern "C"
{
#include <fcntl.h>
}

namespace nfs
{

    NFSUserApp::NFSUserApp() 
    {
        initOperations();
    }
    
    NFSUserApp::~NFSUserApp() 
    {
        if (fd != -1) {
            connection.close(fd);
        }
    }

    void showInfo(std::string info) {
        std::cout << info << "\n";
        std::getchar();
    }

    void showInfo2(std::string info, int arg) {
        std::cout << info << arg << "\n";
        std::getchar();
    }

    void showInvalidCallInfo() {
        showInfo("Invalid call...");    
    }
    
    int NFSUserApp::run() 
    {
        bool exit = false;
        bool connected = false;
        while (!(connected || exit)) {
            showMenu(true);
            stringVector command = parseCommand();
            OperantionCall operantionCall = getOperantionCall(command, true);
            if (operantionCall.name == "connect") {
                if (operantionCall.args.size() == operantionCall.argsValues.size()) {
                    ConnectReturn connectReturn = connection.connect(
                        operantionCall.argsValues[0],
                        operantionCall.argsValues[1],
                        operantionCall.argsValues[2],
                        operantionCall.argsValues[3]
                    );
                    if (connectReturn == OK) {
                        connected = true;
                    } else {
                        showInfo2("Connect operation failed with error code: ", connectReturn);
                    }
                } else {
                    showInvalidCallInfo();
                }
            } else if (operantionCall.name == "exit") {
                exit = true;
            } else {
                showInvalidCallInfo();
            }
        }

        while (!exit && connected) {
            showMenu(false);
            stringVector command = parseCommand();
            OperantionCall operantionCall = getOperantionCall(command, false);
            if (operantionCall.name == "exit") {
                exit = true;
            } else {
                runOperantionCall(operantionCall);
            }
        }
        return 0;
    }
     void NFSUserApp::initOperations() 
    {
        connectOperations.push_back({"connect", {"hostName", "username", "password", "filesystemName"}});
        connectOperations.push_back({"exit", {}});

        operations.push_back({"open", {"path", "oflag", "mode"}});
        operations.push_back({"download file", {"file", "target"}});
        operations.push_back({"send file", {"file", "target"}});
        operations.push_back({"lseek", {"offset", "whence"}});
        operations.push_back({"fstat", {}});
        operations.push_back({"unlink", {"path"}});
        operations.push_back({"flock", {"operation"}});
        operations.push_back({"close", {}});
        operations.push_back({"exit", {}});
    }

    std::string getArgsAsString(Operantion op) {
        std::string args = "";
        for (std::string arg : op.args) {
            args += " <" + arg + ">";
        }
        return args;
    }
    
    void NFSUserApp::showMenu(bool connectMenu) 
    {
        int clearResult = std::system("clear");
        int opId = 1;
        std::cout << "Type operation number or name with args:\n\n";
        std::vector<nfs::Operantion> availableOperations = connectMenu ? connectOperations : operations;
        for (auto op: availableOperations) {
            std::cout << opId << ". " << op.name << " " << getArgsAsString(op) << "\n";
            opId++;
        }
        std::cout << "\n";
    }
    
    stringVector NFSUserApp::parseCommand() 
    {
        std::string command = getCommand();
        std::string delimiter = " ";
        stringVector partsOfCommand{};

        size_t pos = 0;
        while ((pos = command.find(delimiter)) != std::string::npos) {
            std::string token = command.substr(0, pos);
            partsOfCommand.push_back(token);
            command.erase(0, pos + delimiter.length());
        }
        partsOfCommand.push_back(command);
        return partsOfCommand;
    }
    
    std::string NFSUserApp::getCommand() 
    {
        std::string command;
        getline(std::cin, command);
        return command;
    }
    
    OperantionCall NFSUserApp::getOperantionCall(stringVector command, bool allowConnect) 
    {
        if (command.size() <= 0) {
            return { "Invalid call" };
        }
        std::vector<nfs::Operantion> availableOperations = allowConnect ? connectOperations : operations;
        Operantion op;
        std::string operationIdOrName = command[0];
        try {
            int idx = std::stoi(operationIdOrName) - 1;
            op = availableOperations.at(idx);
        } catch (const std::exception& ignore) {
            op.name = "Cannot parse id";
        }
        if (op.name == "Cannot parse id") {
            for (auto operation: availableOperations) {
                if (operation.name == operationIdOrName) {
                    op = operation;
                    break;
                }
            }
            if (op.name == "Cannot parse id") {
                showInfo("Unknown command");
                return { "Invalid call" };
            }
        }
        command.erase(command.begin());
        return { op.name, op.args, command };
    }

    void showStats(struct stat *statbuf) {
        std::cout << "Stats:\n";
        std::cout << "st_dev: " << statbuf->st_dev << "\n";
        std::cout << "st_ino: " << statbuf->st_ino << "\n";
        std::cout << "st_mode: " << statbuf->st_mode << "\n";
        std::cout << "st_nlink: " << statbuf->st_nlink << "\n";
        std::cout << "st_uid: " << statbuf->st_uid << "\n";
        std::cout << "st_gid: " << statbuf->st_gid << "\n";
        std::cout << "st_rdev: " << statbuf->st_rdev << "\n";
        std::cout << "st_size: " << statbuf->st_size << "\n";
        std::cout << "st_blksize: " << statbuf->st_blksize << "\n";
        std::cout << "st_blocks: " << statbuf->st_blocks << "\n";
        std::cout << "st_atim: " << ctime((const time_t*) &(statbuf->st_atim));
        std::cout << "st_mtime: " << ctime((const time_t*) &(statbuf->st_mtime));
        std::cout << "st_ctime: " << ctime((const time_t*) &(statbuf->st_ctime));
        getchar();
    }
    
    int NFSUserApp::runOperantionCall(OperantionCall operantion) 
    {
        if (operantion.args.size() != operantion.argsValues.size()) {
            showInfo("Incorrect number of arguments");
            return -2;
        }
        if (operantion.name == "close") {
            return connection.close(fd);
        }
        if (operantion.name == "open" || 
            operantion.name == "unlink" ||
            operantion.name == "download file" ||
            operantion.name == "send file") {
            const char *path = operantion.argsValues[0].c_str();
            if (operantion.name == "unlink") {
                return connection.unlink(path);
            }
            if (operantion.name == "download file" ||
                operantion.name == "send file") {
                std::string target = operantion.argsValues[1];
                if (operantion.name == "download file") {
                    return downloadFile(path, target);
                } else {
                    return sendFile(path, target);
                }
            }
            try {
                int oflag = std::stoi(operantion.argsValues[1]);
                int mode = std::stoi(operantion.argsValues[2]);
                fd = connection.open(path, oflag, mode);
            } catch (const std::exception& ignore) {
                showInfo("Cannot parse int param");
                return -2;
            }
            return fd;
        }
        
        if (operantion.name == "lseek") {
            try {
                off_t offset = std::stoi(operantion.argsValues[0]);
                int whence = std::stoi(operantion.argsValues[1]);
                offset = connection.lseek(fd, offset, whence);
                showInfo2("File size = ", offset);
            } catch (const std::exception& ignore) {
                showInfo("Cannot parse int param");
                return -2;
            }
        }
        if (operantion.name == "fstat") {
            struct stat statbuf;
            int result = connection.fstat(fd, &statbuf);
            if (result == 0) showStats(&statbuf);
            return result;
        }
        if (operantion.name == "flock") {
            try {
                int operation = std::stoi(operantion.argsValues[0]);
                return connection.flock(fd, operation);
            } catch (const std::exception& ignore) {
                showInfo("Cannot parse int param");
                return -2;
            }
        }
        return -2;
    }
    
    int NFSUserApp::downloadFile(std::string filename, std::string target) 
    {
        fd = connection.open(filename.c_str(), O_RDONLY, 0);
        if (fd <= 0) {
            return 1;
        }
        //find file size with lseek
        auto fileSize = connection.lseek(fd, 0, SEEK_END);
        if (fileSize == (off_t)-1) {
            return 1;
        }
        //lseek back to beginning
        auto seek_back = connection.lseek(fd, 0, SEEK_SET);
        if (seek_back == (off_t)-1) {
            return 1;
        }
        //creat buffer for read data and read
        char *buffer = new char[fileSize];
        int res = connection.read(fd, buffer, fileSize);
        if (res < 0) {
            std::cerr << connection.get_error() << std::endl;
            return 1;
        }
        int res_close = connection.close(fd);
        if (res_close < 0) {
            return 1;
        }
        std::ofstream file(target.c_str(), std::ios::binary);
        file.write(buffer, res);
        delete[] buffer;
        std::cout << "Written " << res << " bytes" << std::endl;
        return 0;
    }

    std::vector<char> readFile(std::string filename)
    {
        std::ifstream file(filename, std::ios::binary);
        file.unsetf(std::ios::skipws);
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> vec;
        vec.reserve(fileSize);
        vec.insert(
            vec.begin(),
            std::istream_iterator<char>(file),
            std::istream_iterator<char>()
        );
        return vec;
    }
    
    int NFSUserApp::sendFile(std::string filename, std::string target) 
    {
        auto file = readFile(filename.c_str());

        fd = connection.open(target.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (fd <= 0) {
            return -1;
        }
        int res = connection.write(fd, file.data(), file.size());
        if (res < 0) {
            return -1;
        }
        int res_close = connection.close(fd);
        if (res_close < 0) {
            return -1;
        }
        showInfo2("Sent ", res);
        return 0;
    }
}