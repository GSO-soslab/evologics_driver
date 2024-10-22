
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

    //strip the first 3 chars, "+++"
    std::string strip = raw.substr(3);

    //find the 'T' and the ':'
    std::string::size_type t_index = strip.find_first_of('T');
    std::string::size_type colon_index = strip.find_first_of(':');

    //the command is from the 'T' to the ':'
    size_t read = colon_index - t_index - 1;
    msg.command = strip.substr(t_index+1, read);

    //the size is from the ':' to the ':'
    std::string::size_type second_colon_index = strip.find_first_of(':', colon_index + 1);
    read = second_colon_index - colon_index - 1;
    int size = std::stoi(strip.substr(colon_index+1, read));

    //the data is from the ':' to the end
    std::string data = strip.substr(second_colon_index + 1);

    int vector_size = 0;
    //if the data contains a comma delimeter then split and push back
    size_t comma_index = data.find_first_of(',');
    if(comma_index != std::string::npos)
    {
        std::stringstream ss;
        ss << data;
        while (ss.good())
        {
            std::string substr;
            getline(ss, substr, ',');
            msg.data.push_back(substr);
            vector_size += 1;

        }
    }
    //else push back all of the data
    else
    {
        msg.data.push_back(data);
    }


    for(auto i: msg.data)
    {
        vector_size += i.size();
    }

    if(cb_)
    {
        if(size == vector_size)
        {
            cb_(msg);   
        }
        else
        {
            printf("PARSED SIZE DOES NOT MATCH ACTUAL SIZE\n");
        }

    }

}
} //namespace hayes
