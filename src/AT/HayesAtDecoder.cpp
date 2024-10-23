
/*
    Author: Jason Miller, jason_miller@uri.edu
    Year: 2023

    Copyright (C) 2023 Smart Ocean Systems Laboratory
*/

#include "HayesAtDecoder.h"

namespace hayes
{

AtDecoder::AtDecoder()
{
}

void AtDecoder::decode(const std::string &raw)
{

    if (raw.empty())
        throw "Bad decode";

    hayes::AtMsg msg;

    size_t comma_index = raw.find_first_of(',');
    if(comma_index != std::string::npos)
    {
        size_t t_index = raw.find_first_of('T');
        size_t colon_index = raw.find_first_of(':', t_index+2);
        int read = comma_index - colon_index - 1;
        msg.command = raw.substr(colon_index+1, read);
        std::string data = raw.substr(comma_index+1);
        std::stringstream ss;
        ss << data;
        while (ss.good())
        {
            std::string substr;
            getline(ss, substr, ',');
            msg.data.push_back(substr);
        }
    }
    else
    {
        msg.command = raw;
    }



    if(cb_)
    {
        cb_(msg);   
    }

}
} //namespace hayes
