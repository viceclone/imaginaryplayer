#pragma once

#include <iostream>

#define BOLD(s) "\x1B[1m" s "\x1B[00m"
#define ITALIC(s) "\x1B[3m" s "\x1B[00m"

#define RED(s) "\x1B[31m" s "\x1B[00m"
#define GREEN(s) "\x1B[32m" s "\x1B[00m"
#define YELLOW(s) "\x1B[33m" s "\x1B[00m"
#define CYAN(s) "\x1B[36m" s "\x1B[00m"

#define NEWLINE() std::cout << std::endl

#define LOG_COMMAND(command) std::cout << ">>>>> Command: " << BOLD(command) << "<<<<<" << std::endl
#define LOG(s) std::cout << ITALIC(s) << std::endl