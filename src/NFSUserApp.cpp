#include "../include/NFSUserApp.hpp"

namespace nfs
{

    NFSUserApp::NFSUserApp() 
    {
        initOperations();
    }
    
    NFSUserApp::~NFSUserApp() 
    {
        connection.close(fd);
    }

    void showInfo(std::string info) {
        std::cout << info << "\n";
        std::getchar();
    }

    void showInvalidCallInfo() {
        showInfo("Invalid call...");    
    }
    
    int NFSUserApp::run() 
    {
        bool exit = false;
        bool connected = false;
        while (!connected || !exit) {
            showConnectionMenu();
            stringVector command = parseCommand();
            OperantionCall operantionCall = getOperantionCall(command, true);
            if (operantionCall.name == "connect") {
                if (operantionCall.args.size() == operantionCall.argsValues.size()) {
                    ConnectReturn connectReturn = connection.connect(
                        command[0],
                        command[1],
                        command[2],
                        command[3]
                    );
                    if (connectReturn == OK) {
                        connected = true;
                    } else {
                        showInfo("Connect operation failed with error code: " + connectReturn);
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
            showMenu();
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
        connectOperation.name = "connect";
        connectOperation.args = {"hostName", "username", "password", "filesystemName"};
        operations.push_back({"open", {"path", "oflag", "mode"}});
        operations.push_back({"read", {"path"}});
        operations.push_back({"write", {"path"}});
        operations.push_back({"lseek", {"offset", "whence"}});
        operations.push_back({"fstat", {"statbuf"}});
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

    void NFSUserApp::showConnectionMenu() 
    {
        std::system("clear");
        std::cout << "Type operation number or name with args:\n\n1. connect <>\n2. exit\n\n";
    }
    
    
    void NFSUserApp::showMenu() 
    {
        std::system("clear");
        int opId = 1;
        std::cout << "Type operation number or name with args:\n\n";
        for (auto op: operations) {
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
            partsOfCommand.push_back(command.substr(0, pos));
            command.erase(0, pos + delimiter.length());
        }

        for (const auto &str : partsOfCommand) {
            std::cout << str << "\n";
        }
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
        Operantion op;
        std::string operationIdOrName = command[0];
        try {
            int idx = std::stoi(operationIdOrName);
            op = operations.at(idx);
        } catch (const std::exception& ignore) {
            op.name = { "Invalid call" };
        }
        if (op.name == "Invalid call") {
            for (auto operation: operations) {
                if (operation.name == operationIdOrName) {
                    op = operation;
                    break;
                }
            }
            if (allowConnect) {

            }
            if (op.name == "Invalid call") {
                return { "Invalid call" };
            }
        }
        command.erase(command.begin());
        return { op.name, op.args, command };
    }
    
    int NFSUserApp::runOperantionCall(OperantionCall operantion) 
    {
        // if (operantion.args.size() != operantion.argsValues.size()) {
        //     return INVALID_CALL;
        // }
        // if (op.name == "close") {
        //     return connection.close(fd);
        // }
        // if (op.name == "open") {
        //     try {
        //         int idx = std::stoi(operationIdOrName);
        //         op = operations.at(idx);
        //     } catch (const std::exception& ignore) {
        //         op.name = { "Invalid call" };
        //     }
        //     fd = connection.open(path, oflag, mode);
        //     return OK;
        // }
        // if (op.name == "read") {
        //     return connection.read(fd, buf, count);
        // }
        // if (op.name == "write") {
        //     return connection.write(fd, buf, count);
        // }
        // if (op.name == "lseek") {
        //     return connection.lseek(fd, offset, whence);
        // }
        // if (op.name == "fstat") {
        //     return connection.fstat(fd, statbuf);
        // }
        // if (op.name == "unlink") {
        //     return connection.unlink(path);
        // }
        // if (op.name == "flock") {
        //     return connection.flock(fd, operation);
        // }
        return -1;
    }
}