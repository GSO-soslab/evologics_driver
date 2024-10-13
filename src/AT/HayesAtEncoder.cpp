/*
    Author: Jason Miller, jason_miller@uri.edu
    Year: 2023

    Copyright (C) 2023 Smart Ocean Systems Laboratory
*/

#include "HayesAtEncoder.h"

namespace hayes
{

AtEncoder::AtEncoder()
{
}

void AtEncoder::encode(const AtMsg &at_msg)
{
    std::stringstream message;

    message << "+++AT" << at_msg.command;

    for (size_t i = 0; i < at_msg.data.size(); i++)
    {
        message << "," << at_msg.data.at(i);
    }

    cb_(message.str());
}
} //namespace hayes
