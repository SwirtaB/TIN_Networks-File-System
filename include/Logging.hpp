#pragma once

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>

namespace nfs
{

static const bool LOGGING = true;

template <class... Args>
void log_cerr(Args... args) {
    if (LOGGING) {
        auto now          = std::chrono::system_clock::now();
        auto current_time = std::chrono::system_clock::to_time_t(now);
        auto time         = std::localtime(&current_time);

        char buffer[1000];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time);

        std::cerr << buffer << " ";

        ([&](auto &arg) { std::cerr << arg; }(args), ...);

        std::cerr << std::endl;
    }
}

template <class T>
void log_perror(T msg) {
    if (LOGGING) {
        auto now          = std::chrono::system_clock::now();
        auto current_time = std::chrono::system_clock::to_time_t(now);
        auto time         = std::localtime(&current_time);

        char buffer[1000];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time);

        std::cerr << buffer << " ";
        perror(msg);
    }
}

} // namespace nfs