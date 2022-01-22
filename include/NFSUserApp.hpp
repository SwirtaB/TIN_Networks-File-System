#pragma once

#include "NFSConnection.hpp"
#include <string>
#include <list>
#include <vector>
#include <iostream>

namespace nfs
{
    typedef std::vector<std::string> stringVector;

    struct Operantion {
        std::string name;
        stringVector args;
    };

    struct OperantionCall: Operantion {
        stringVector argsValues;
    };

    class NFSUserApp
    {
    public:
        NFSUserApp();
        ~NFSUserApp();

        int run();

    private:
        Operantion connectOperation;
        std::vector<Operantion> operations;

        NFSConnection connection;
        int fd = -1;

        void initOperations();
        void showConnectionMenu();
        void showMenu();
        stringVector parseCommand();
        std::string getCommand();
        OperantionCall getOperantionCall(stringVector command, bool allowConnect);
        int runOperantionCall(OperantionCall operantionCall);
    };

} // namespace nfs