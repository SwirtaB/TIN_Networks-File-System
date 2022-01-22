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
        while (!(connected || exit)) {
            showMenu(true);
            stringVector command = parseCommand();
            OperantionCall operantionCall = getOperantionCall(command, true);
            std::cout << "operationCall name = " << operantionCall.name << "\n";
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
    
    void NFSUserApp::showMenu(bool connectMenu) 
    {
        std::system("clear");
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
        std::vector<nfs::Operantion> availableOperations = allowConnect ? connectOperations : operations;
        Operantion op;
        std::string operationIdOrName = command[0];
        try {
            int idx = std::stoi(operationIdOrName) - 1;
            op = availableOperations.at(idx);
        } catch (const std::exception& ignore) {
            op.name = { "Invalid call" };
        }
        if (op.name == "Invalid call") {
            for (auto operation: availableOperations) {
                if (operation.name == operationIdOrName) {
                    op = operation;
                    break;
                }
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