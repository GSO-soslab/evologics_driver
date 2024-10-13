
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


    std::string::size_type read_position = raw.find_first_of('?');
    std::string::size_type write_position = raw.find_first_of('!');

    std::string split_str = "+++AT:";
    size_t split_index = raw.find(split_str);

    std::string strip_str = raw.substr(split_index + split_str.size());

    size_t colon_index = strip_str.find_first_of(':');
    std::string data_str;

    if(read_position != std::string::npos || write_position != std::string::npos)
    {

        msg.command = strip_str.substr(0,colon_index);

        strip_str = strip_str.substr(colon_index+1);

        colon_index = strip_str.find_first_of(':');

        data_str = strip_str.substr(colon_index+1);
    }
    else
    {
        size_t comma_index = strip_str.find_first_of(',', colon_index+1);
        size_t read = comma_index - colon_index - 1;
        msg.command = strip_str.substr(colon_index+1, read);

        data_str = strip_str.substr(comma_index);
    }

    size_t delimeter_index = data_str.find_first_of(',');

    if(delimeter_index == std::string::npos)
    {
        msg.data.push_back(data_str);
    }
    else
    {
        while (delimeter_index != std::string::npos)
        {
            // Delimeter is pointing to the comma at the start of this field.

            // Find the index of the next comma.
            size_t next_delimiter_index = data_str.find_first_of(',', delimeter_index + 1);

            // Following comma found, set length using position of next comma.
            size_t read_length = next_delimiter_index - delimeter_index - 1;

            // Pull substring into field.
            msg.data.push_back(data_str.substr(delimeter_index + 1, read_length));

            // Update delimeter index.
            delimeter_index = next_delimiter_index;
        }
    }

    if(cb_)
    {
        cb_(msg);
    }

}
} //namespace hayes
