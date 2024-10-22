
#include <algorithm>   // for copy, max
#include <cerrno>      // for errno
#include <chrono>      // for seconds
#include <cmath>       // for abs
#include <cstdlib>     // for abs
#include <cstring>     // for strerror
#include <iterator>    // for ostrea...
#include <list>        // for operat_...
#include <locale>      // for locale
#include <memory>      // for allocator
#include <sstream>     // for basic_...
#include <stdexcept>   // for out_of...
#include <sys/ioctl.h> // for ioctl
#include <unistd.h>    // for usleep
#include <utility>     // for pair
#include <vector>      // for vector

#include <boost/algorithm/string/case_conv.hpp>      // for to_upp...
#include <boost/algorithm/string/classification.hpp> // for is_any...
#include <boost/algorithm/string/split.hpp>          // for split
#include <boost/algorithm/string/trim.hpp>           // for trim
#include <boost/date_time/date.hpp>                  // for date<>...
#include <boost/date_time/gregorian/gregorian.hpp>   // for date
#include <boost/date_time/posix_time/ptime.hpp>      // for ptime
#include <boost/date_time/time.hpp>                  // for base_t...
#include <boost/signals2/signal.hpp>                 // for signal
#include <dccl/binary.h>                             // for b64_de...
#include <dccl/codec.h>                              // for Codec
#include <dccl/common.h>                             // for operat_...

#include "goby/acomms/acomms_constants.h"                // for BROADC...
#include "goby/acomms/protobuf/modem_driver_status.pb.h" // for ModemD...
#include "goby/time/convert.h"                           // for System...
#include "goby/time/types.h"                             // for MicroTime
#include "goby/util/as.h"                                // for as
#include "goby/util/binary.h"                            // for hex_de...
#include "goby/util/debug_logger/flex_ostream.h"         // for FlexOs...
#include "goby/util/debug_logger/flex_ostreambuf.h"      // for DEBUG1
#include "goby/util/debug_logger/logger_manipulators.h"  // for operat_...
#include "goby/util/debug_logger/term_color.h"           // for magenta
#include "goby/util/linebasedcomms/interface.h"          // for LineBa...
#include "goby/util/linebasedcomms/serial_client.h"      // for Serial...
#include "goby/util/protobuf/io.h"                       // for operat_...

#include "goby/acomms/modemdriver/driver_exception.h" // for ModemD...

#include "evologics_driver.h"

using goby::glog;
using goby::util::as;
using goby::util::hex_decode;
using goby::util::hex_encode;
using namespace goby::util::tcolor;
using namespace goby::util::logger;
using namespace goby::util::logger_lock;

const std::string goby::acomms::EvologicsDriver::SERIAL_DELIMITER = "\r\n";
const std::string goby::acomms::EvologicsDriver::ETHERNET_DELIMITER = "\r\n";

goby::acomms::EvologicsDriver::EvologicsDriver()
{

    encoder_.set_transmit_callback(
        std::bind(&EvologicsDriver::config_write, this, std::placeholders::_1));

    decoder_.set_decode_callback(
        std::bind(&EvologicsDriver::on_decode, this, std::placeholders::_1));
}

goby::acomms::EvologicsDriver::~EvologicsDriver() = default;

void goby::acomms::EvologicsDriver::startup(const protobuf::DriverConfig& cfg)
{

    glog.is(DEBUG1) && glog << group(glog_out_group()) << "Goby Evologics driver starting up."
                            << std::endl;

    if (startup_done_)
    {
        glog.is(DEBUG1) && glog << group(glog_out_group())
                                << " ... driver is already started, not restarting." << std::endl;
        return;
    }

    // store a copy for us later
    driver_cfg_ = cfg;


    // setup cfg based on serial or ethernet
    // no default because modem_start will catch if no connection type is set and print debug statement
    switch(cfg.connection_type())
    {
        case protobuf::DriverConfig::CONNECTION_SERIAL:
            driver_cfg_.set_line_delimiter(SERIAL_DELIMITER);

            if (!cfg.has_serial_baud())
                driver_cfg_.set_serial_baud(DEFAULT_BAUD);

            break;

        case protobuf::DriverConfig::CONNECTION_TCP_AS_CLIENT:
            driver_cfg_.set_line_delimiter(ETHERNET_DELIMITER);

            if(!cfg.has_tcp_server())
                driver_cfg_.set_tcp_server(DEFAULT_TCP_SERVER);

            if(!cfg.has_tcp_port())
                driver_cfg_.set_tcp_port(DEFAULT_TCP_PORT);

            break;

        default:
            break;

    }

    modem_start(driver_cfg_);

    clear_buffer();

    startup_done_ = true;
}

void goby::acomms::EvologicsDriver::clear_buffer()
{
    hayes::AtMsg msg;
    msg.command = "Z4";
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::shutdown()
{
    ModemDriverBase::modem_close();
}

void goby::acomms::EvologicsDriver::extended_notification_on()
{
    hayes::AtMsg msg;
    msg.command = "@ZX1";
    encoder_.encode(msg);   
}
void goby::acomms::EvologicsDriver::extended_notification_off()
{
    hayes::AtMsg msg;
    msg.command = "@ZX0";
    encoder_.encode(msg);   
}
void goby::acomms::EvologicsDriver::set_source_level(int source_level)
{
    hayes::AtMsg msg;
    msg.command = "!L" + std::to_string(source_level);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_source_control(int source_control)
{
    hayes::AtMsg msg;
    msg.command = "!LC" + std::to_string(source_control);
    encoder_.encode(msg);

}

void goby::acomms::EvologicsDriver::set_gain(int gain)
{
    hayes::AtMsg msg;
    msg.command = "!G" + std::to_string(gain);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_carrier_waveform_id(int id)
{
    hayes::AtMsg msg;
    msg.command = "!C" + std::to_string(id);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_local_address(int address)
{
    hayes::AtMsg msg;
    msg.command = "!AL" + std::to_string(address);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_remote_address(int address)
{
    hayes::AtMsg msg;
    msg.command = "!AR" + std::to_string(address);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_highest_address(int address)
{
    hayes::AtMsg msg;
    msg.command = "!AM" + std::to_string(address);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_cluster_size(int size)
{
    hayes::AtMsg msg;
    msg.command = "!ZC" + std::to_string(size);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_packet_time(int time)
{
    hayes::AtMsg msg;
    msg.command = "!ZP" + std::to_string(time);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_retry_count(int count)
{
    hayes::AtMsg msg;
    msg.command = "!RC" + std::to_string(count);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_retry_timeout(int time)
{
    hayes::AtMsg msg;
    msg.command = "!RT" + std::to_string(time);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_keep_online_count(int count)
{
    hayes::AtMsg msg;
    msg.command = "!KO" + std::to_string(count);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_idle_timeout(int time)
{
    hayes::AtMsg msg;
    msg.command = "!ZI" + std::to_string(time);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_channel_protocol_id(int id)
{
    hayes::AtMsg msg;
    msg.command = "!ZS" + std::to_string(id);
    encoder_.encode(msg);
}

void goby::acomms::EvologicsDriver::set_sound_speed(int speed)
{
    hayes::AtMsg msg;
    msg.command = "!CA" + std::to_string(speed);
    encoder_.encode(msg);
}


void goby::acomms::EvologicsDriver::do_work()
{

    // read any incoming messages from the modem
    std::string raw_str;
    while (modem_read(&raw_str))
    {

        //Pop the last two bytes because they are the \r\n delimeter
        raw_str.pop_back();
        raw_str.pop_back();

        // try to handle the received message, posting appropriate signals
        try
        {
            std::string split_str = "+++";
            size_t split_index = raw_str.find(split_str);

            //if it is not an AT message, then it is comms data
            if(split_index == std::string::npos)
            {
                glog.is(DEBUG1) && glog << group(glog_out_group()) << "BINARY RX: " << hex_encode(raw_str.c_str()) << std::endl;
                process_receive(raw_str);
            }
            //it is an AT message
            else
            {
                glog.is(DEBUG1) && glog << group(glog_out_group()) << "AT COMMAND RX: " << raw_str.c_str() << std::endl;
                decoder_.decode(raw_str);
            }
        }
        catch (std::exception& e)
        {
            glog.is(DEBUG1) && glog << group(glog_in_group()) << warn
                                    << "Failed to handle message: " << e.what() << std::endl;
        }   
    }

}   

void goby::acomms::EvologicsDriver::process_receive(const std::string& s)
{
    protobuf::ModemRaw raw_msg;
    raw_msg.set_raw(s);

    signal_raw_incoming(raw_msg);

    receive_msg_.add_frame(s);

    signal_receive_and_clear(&receive_msg_);
    

} 

void goby::acomms::EvologicsDriver::handle_initiate_transmission(const protobuf::ModemTransmission& msg)
{
    transmit_msg_.CopyFrom(msg);

    try
    {
        // glog.is(DEBUG1) && glog << group(glog_out_group()) << "We were asked to transmit from "
        //                         << msg.src() << " to " << msg.dest() << std::endl;

        signal_modify_transmission(&transmit_msg_);

        switch(transmit_msg_.type())
        {
            case protobuf::ModemTransmission::DATA:
            {
                transmit_msg_.set_max_frame_bytes(1000);
                signal_data_request(&transmit_msg_);
                data_transmission(&transmit_msg_); 
                
            } 
            break;

            default:
                glog.is(DEBUG1) && glog << group(glog_out_group()) << warn
                                        << "Not initiating transmission because we were given an "
                                           "invalid transmission type for the base Driver:"
                                        << transmit_msg_ << std::endl;
                break;
        }
    }
    catch (ModemDriverException& e)
    {
        glog.is(DEBUG1) && glog << group(glog_out_group()) << warn
                                << "Failed to initiate transmission: " << e.what() << std::endl;
    }
}

void goby::acomms::EvologicsDriver::data_transmission(protobuf::ModemTransmission* msg)
{
    //do we need to set max frames and bytes for data mode?
    msg->set_max_num_frames(1);
    msg->set_max_frame_bytes(1000);

    if (!(msg->frame_size() == 0 || msg->frame(0).empty()))
    {
        
        evologics_write(msg->frame(0));

    }
    else
    {
        glog.is(DEBUG1) && glog << group(glog_out_group())
                                << "Not initiating transmission because we have no data to send"
                                << std::endl;
    }
}

void goby::acomms::EvologicsDriver::evologics_write(const std::string &s)
{
    protobuf::ModemRaw raw_msg;
    raw_msg.set_raw(s);
                            
    signal_raw_outgoing(raw_msg);

    glog.is(DEBUG1) && glog << group(glog_out_group()) << "BINARY TX: " << hex_encode(s.c_str()) << std::endl;

    if(driver_cfg_.connection_type() == protobuf::DriverConfig::CONNECTION_SERIAL)
    {
        modem_write(raw_msg.raw()+"\r\n");
    }
    else if(driver_cfg_.connection_type() == protobuf::DriverConfig::CONNECTION_TCP_AS_CLIENT)
    {
        modem_write(raw_msg.raw()+"\r\n");
    }
}

void goby::acomms::EvologicsDriver::config_write(const std::string &s)
{
    protobuf::ModemRaw raw_msg;
    raw_msg.set_raw(s);
                            
    signal_raw_outgoing(raw_msg);

    glog.is(DEBUG1) && glog << group(glog_out_group()) << "CONFIG TX: " << s.c_str() << std::endl;

    if(driver_cfg_.connection_type() == protobuf::DriverConfig::CONNECTION_SERIAL)
    {
        modem_write(raw_msg.raw()+"\r\n");
    }
    else if(driver_cfg_.connection_type() == protobuf::DriverConfig::CONNECTION_TCP_AS_CLIENT)
    {
        modem_write(raw_msg.raw()+"\n");
    }
}

void goby::acomms::EvologicsDriver::on_decode(const hayes::AtMsg msg)
{

    if(msg.command == "USBLLONG")
    {
        UsbllongMsg usbl;

        usbl.current_time = std::stof(msg.data[0]);
        usbl.measurement_time = std::stof(msg.data[1]);
        usbl.remote_address = std::stoi(msg.data[2]);
        usbl.xyz.x = std::stof(msg.data[3]);
        usbl.xyz.y = std::stof(msg.data[4]);
        usbl.xyz.z = std::stof(msg.data[5]);
        usbl.enu.e = std::stof(msg.data[6]);
        usbl.enu.n = std::stof(msg.data[7]);
        usbl.enu.u = std::stof(msg.data[8]);
        usbl.rpy.roll = std::stof(msg.data[9]);
        usbl.rpy.pitch = std::stof(msg.data[10]);
        usbl.rpy.yaw = std::stof(msg.data[11]);
        usbl.propogation_time = std::stof(msg.data[12]);
        usbl.rssi = std::stoi(msg.data[13]);
        usbl.integrity = std::stoi(msg.data[14]);
        usbl.accuracy = std::stof(msg.data[15]);

        if(usbl_callback_)
        {
            usbl_callback_(usbl);
        }
    }
    if(msg.command == "SENDSTART")
    {
        if(transmit_callback_)
        {
            transmit_callback_(true);
        }
    }
    if(msg.command == "SENDEND")
    {
        if(transmit_callback_)
        {
            transmit_callback_(false);
        }
    }


}

void goby::acomms::EvologicsDriver::signal_receive_and_clear(protobuf::ModemTransmission* message)
{
    try
    {
        signal_receive(*message);
        message->Clear();
    }
    catch (std::exception& e)
    {
        message->Clear();
        throw;
    }
}