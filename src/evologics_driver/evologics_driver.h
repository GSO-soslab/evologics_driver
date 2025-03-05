/*
    Author: Jason Miller, jason_miller@uri.edu
    Year: 2023

    Copyright (C) 2023 Smart Ocean Systems Laboratory
*/

#ifndef GOBY_ACOMMS_MODEMDRIVER_EVO_DRIVER_H
#define GOBY_ACOMMS_MODEMDRIVER_EVO_DRIVER_H

#include <cstdint> // for uint32_t
#include <deque>    // for deque
#include <map>      // for map
#include <memory>   // for unique_ptr
#include <mutex>    // for mutex
#include <set>      // for set
#include <string>   // for string

#include "goby/acomms/modemdriver/driver_base.h"    // for ModemDriverBase
#include "goby/acomms/protobuf/driver_base.pb.h"    // for DriverConfig
#include "goby/acomms/protobuf/modem_message.pb.h"  // for ModemTransmission
#include "goby/time/system_clock.h"                 // for SystemClock, Sys...

#include "HayesAtEncoder.h"
#include "HayesAtDecoder.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>

namespace dccl
{
class Codec;
} // namespace dccl

namespace goby
{
namespace acomms
{
/// \class EvologicsDriver evologics_driver.h goby/acomms/modem_driver.h
/// \ingroup acomms_api
/// \brief provides an API to the Evologics Modem driver
class EvologicsDriver : public ModemDriverBase
{
  public:
      struct XYZ
  {
      float x;
      float y;
      float z;
  };

  struct ENU
  {
      float e;
      float n;
      float u;
  };

  struct RPY
  {
      float roll;
      float pitch;
      float yaw;
  };

  struct UsbllongMsg
  {
      double current_time;
      double measurement_time;
      int remote_address;
      XYZ xyz;
      ENU enu;
      RPY rpy;
      float propogation_time;
      int rssi;
      int integrity;
      float accuracy;
  };

  struct UsblAnglesMsg
  {
    float current_time;
    float measurement_time;
    int remote_address;
    float local_bearing;
    float local_elevation;
    float bearing;
    float elevation;
    float roll;
    float pitch;
    float yaw;
    float rssi;
    float integrity;
    float accuracy;
  };

  struct UsblPhydMsg
  {
    float current_time;
    float measurement_time;
    int remote_address;
    bool fix_type;
    int delay_1_5;
    int delay_2_5;
    int delay_3_5;
    int delay_4_5;
    int delay_1_2;
    int delay_4_1;
    int delay_3_2;
    int delay_3_4;
  };


    typedef std::function<void(UsbllongMsg)> UsblCallback;
    UsblCallback usbl_callback_;

    typedef std::function<void(bool)> TransmitCallback;
    TransmitCallback transmit_callback_;

    typedef std::function<void(UsblAnglesMsg)> AnglesCallback;
    AnglesCallback angles_callback_;

    typedef std::function<void(UsblPhydMsg)> PhydCallback;
    PhydCallback phyd_callback_;


    /// \brief Default constructor.
    EvologicsDriver();

    /// Destructor.
    ~EvologicsDriver() override;

    /// \brief Starts the driver.
    ///
    /// \param cfg Configuration for the Micro-Modem driver. DriverConfig is defined in acomms_driver_base.proto.
    void startup(const protobuf::DriverConfig& cfg) override;

    void clear_buffer();

    /// \brief Stops the driver.
    void shutdown() override;

    /// \brief See ModemDriverBase::do_work()
    void do_work() override;

    /// \brief See ModemDriverBase::handle_initiate_transmission()
    void handle_initiate_transmission(const protobuf::ModemTransmission& m) override;

    bool is_started() const { return startup_done_; }

    void extended_notification_on();

    void extended_notification_off();

    void set_source_level(int source_level);

    void set_source_control(int source_control);

    void set_gain(int gain);

    void set_carrier_waveform_id(int id);

    void set_local_address(int address);

    void set_remote_address(int address);

    void set_highest_address(int address);

    void set_cluster_size(int size);

    void set_packet_time(int time);

    void set_retry_count(int count);

    void set_retry_timeout(int time);

    void set_keep_online_count(int count);

    void set_idle_timeout(int time);

    void set_channel_protocol_id(int id);

    void set_sound_speed(int speed);

    void save_settings();

    void factory_reset();

    void set_usbl_callback(UsblCallback c) { usbl_callback_  = c;}

    void set_transmit_callback(TransmitCallback c) { transmit_callback_ = c;}

    void set_angles_callback(AnglesCallback c){ angles_callback_ = c;}

    void set_phyd_callback(PhydCallback c){ phyd_callback_ = c;}


    // output
    void evologics_write(const std::string &s); // actually write a message
    void config_write(const std::string &s); // actually write a message
    void on_decode(const hayes::AtMsg msg);
    void data_transmission(protobuf::ModemTransmission *msg);

    // input
    void process_receive(const std::string& in); // parse a receive message and call proper method
    void process_at_receive(const std::string& in);

    void signal_receive_and_clear(protobuf::ModemTransmission* message);

    

  private:
    // for the serial connection
    std::string buffer_;

    std::string DEFAULT_TCP_SERVER = "192.168.0.209";
    int DEFAULT_TCP_PORT = 9200;
    int DEFAULT_BAUD = 19200;

    static const std::string SERIAL_DELIMITER;
    static const std::string ETHERNET_DELIMITER;

    // all startup configuration (DriverConfig defined in acomms_driver_base.proto and extended in acomms_mm_driver.proto)
    protobuf::DriverConfig driver_cfg_;

    // set after the startup routines finish once. we can't startup on instantiation because
    // the base class sets some of our references (from the MOOS file)
    bool startup_done_{false};

    protobuf::ModemTransmission transmit_msg_;
    protobuf::ModemTransmission receive_msg_;

    std::unique_ptr<dccl::Codec> dccl_;
    // DCCL requires full memory barrier...
    static std::mutex dccl_mutex_;

    hayes::AtEncoder encoder_;
    hayes::AtDecoder decoder_;



};
} // namespace acomms
} // namespace goby
#endif