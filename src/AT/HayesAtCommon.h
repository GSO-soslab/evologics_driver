#ifndef HAYES_AT_COMMON_H
#define HAYES_AT_COMMON_H

#include <string>
#include <vector>

namespace hayes
{
    struct AtMsg
    {
        std::string command;

        std::vector<std::string> data;
    };
}


#endif