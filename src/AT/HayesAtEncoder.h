#ifndef HAYES_AT_ENCODER_H
#define HAYES_AT_ENCODER_H

/*
    Author: Jason Miller, jason_miller@uri.edu
    Year: 2023

    Copyright (C) 2023 Smart Ocean Systems Laboratory
*/
#include <sstream>
#include <functional>
#include "HayesAtCommon.h"

namespace hayes
{
class AtEncoder
{
public:
    AtEncoder();
    void encode(const AtMsg &at_msg);

    typedef std::function<void(std::string)> TransmitCallback;
    TransmitCallback cb_;

    void set_transmit_callback(TransmitCallback c) { cb_  = c;}

private:


};
} //namespace at

#endif
