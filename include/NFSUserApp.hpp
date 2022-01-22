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
        std::vector<Operantion> connectOperations;
        std::vector<Operantion> operations;

        NFSConnection connection;
        int fd = -1;

        void initOperations();
        void showMenu(bool connectMenu);
        stringVector parseCommand();
        std::string getCommand();
        OperantionCall getOperantionCall(stringVector command, bool allowConnect);
        int runOperantionCall(OperantionCall operantionCall);
    };

} // namespace nfs