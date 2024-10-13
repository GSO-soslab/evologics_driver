#ifndef HAYES_AT_DECODER_H
#define HAYES_AT_DECODER_H

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

class AtDecoder
{
public:
    AtDecoder();
    void decode(const std::string &);

    typedef std::function<void(AtMsg)> DecodeCallback;
    DecodeCallback cb_;

    void set_decode_callback(DecodeCallback c) { cb_  = c;}


private:


};
} //namespace hayes

#endif