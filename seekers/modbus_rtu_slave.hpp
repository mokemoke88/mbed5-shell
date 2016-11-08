/**
 * @file modbus_rtu_slave.hpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-04
 * @par history
 * - 2016-11-04 11:32:53
 *  - First.
 */

#ifndef SEEKERS_MODBUS_RTU_SLAVE_HPP
#define SEEKERS_MODBUS_RTU_SLAVE_HPP

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

class modbus_rtu_slave : public basic_com_module{
private:
  Timer idle_timer_;

  uint8_t adr_;
  std::vector<uint8_t> rx_buff_;
  std::vector<uint8_t> tx_buff_;

  static uint16_t crc16(const uint8_t* data, size_t size)
  {
    return crc16_ibm(data, size);
  }

  typedef std::vector<uint8_t>::iterator Iter;
  int idle_limit_;

#ifdef __MBED__
#ifndef NDEBUG
  RawSerial& debug_;
#endif
#endif

  static void exceptionresponse(std::vector<uint8_t>& /*dst*/, uint8_t /*adr*/, uint8_t /*cmd*/, uint8_t /*code*/);

  bool readcoilstatus_(void);
  bool readinputstatus_(void);
  bool readholdingregister_(void);
  bool readinputregister_(void);
  bool forcesinglecoil_(void);

protected:

  virtual void readcoilstatus(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt);
  virtual void readinputstatus(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt);
  virtual void readholdingregister(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt);
  virtual void readinputregister(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt);
  virtual void forcesinglecoil(std::vector<uint8_t>&dst, uint16_t start_adr, uint16_t value);

public:

#ifndef NDEBUG
  modbus_rtu_slave(RawSerial& debug, uint8_t adr = 1) :
    adr_(adr),
    idle_limit_(4),
    debug_(debug)
#else
  modbus_rtu_slave(uint8_t adr = 1) :
    adr_(adr),
    idle_limit_(4)
#endif
  {
    idle_timer_.start();
  }

  virtual ~modbus_rtu_slave(){}

  /**
   * @brief 受信処理
   */
  void recieve(std::vector<uint8_t>& tx_buff, const uint8_t* src, size_t size);

  /**
   * @brief アイドル動作
   */
  void idle(std::vector<uint8_t>& tx_buff);
};

} /* namespace */


#endif /* SEEKERS_MODBUS_RTU_SLAVE_HPP */
