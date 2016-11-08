/**
 * @file modbus_rtu_master.hpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-05
 * @par history
 * - 2016-11-05 08:41:11
 *  - First.
 */

#ifndef SEEKERS_MODBUS_RTU_MASTER_HPP
#define SEEKERS_MODBUS_RTU_MASTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>

#ifdef __MBED__
#include "mbed.h"
#else
#endif

#include "utils.hpp"
#include "basic_com_module.hpp"

namespace seekers{

/**
 * @brief MODBUS 汎用
 */
class modbus{
private:
  modbus();
public:
  static void request_readcoilstatus(std::vector<uint8_t>& dst, uint8_t slave, uint16_t reg_adr, uint16_t reg_cnt);
};

/**
 * @brief MODBUS RTU マスター側
 */
class modbus_rtu_master : public basic_com_module{
public:

  typedef void (*response_handler_t)( modbus_rtu_master*, const uint8_t*, size_t );
  typedef void (*response_timeout_handler_t)( modbus_rtu_master*, uint8_t, uint8_t );

  enum handler_type_t{
    READCOILSTATUS,
    EXCEPTIONRESPONSE
  };

private:
  enum stat_t{
    STAT_HALT,
    STAT_WAIT_FOR_REQUEST
  };

  stat_t stat_;
  Timer idle_timer_;
  Timer response_timer_;

  int idle_limit_;
  int response_limit_;

  uint8_t tgt_slave_;
  uint8_t tgt_cmd_;

  std::vector<uint8_t> rx_buff_;

  uint16_t crc16(const uint8_t* src, size_t size){
    return seekers::crc16_ibm(src, size);
  }

#ifdef __MBED__
#ifndef NDEBUG
  mbed::Stream& debug_;
#endif
#endif

  response_timeout_handler_t response_timeout_handler_;

  response_handler_t exceptionresponse_handler_;
  response_handler_t readcoilstatus_handler_;

  bool exceptionresponse_(void);
  bool readcoilstatus_(void);

public:
  void request_readcoilstatus(std::vector<uint8_t>& dst, uint8_t slave, uint16_t reg_adr, uint16_t reg_cnt);
  void idle(std::vector<uint8_t>& dst);

  void recieve(std::vector<uint8_t>& tx_buff, const uint8_t* src, size_t size);
  void sethandler(response_handler_t handler, handler_type_t handler_type);
  void settimeout_handler(response_timeout_handler_t handler)
  {
    response_timeout_handler_ = handler;
  }

public:
  modbus_rtu_master(mbed::Stream& debug) :
    stat_(STAT_HALT),
    idle_limit_(4),
    response_limit_(500),
    tgt_slave_(0x00),
    tgt_cmd_(0x00),
    debug_(debug),
    response_timeout_handler_(NULL),
    exceptionresponse_handler_(NULL),
    readcoilstatus_handler_(NULL)
  {
    idle_timer_.start();
    response_timer_.start();
  }
};


} /* namespace */

#endif /* SEEKERS_MODBUS_RTU_MASTER_HPP */
