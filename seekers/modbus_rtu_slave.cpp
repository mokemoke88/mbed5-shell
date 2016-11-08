/**
 * @file modbus_rtu_slave.cpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-04
 * @par history
 * - 2016-11-04 11:46:59
 *  - first.
 */


#include "modbus_rtu_slave.hpp"

namespace seekers{

/**
 * @brief データ受信時の処理
 */
void modbus_rtu_slave::recieve(std::vector<uint8_t>& tx_buff, const uint8_t* src, size_t size)
{
#ifndef NDEBUG
  // debug_.printf("[DEBUG] modbus_rtu_slave[%d] recieve()\r\n", adr_);
#endif
  if(idle_timer_.read_ms() >= idle_limit_ ){
#ifndef NDEBUG
    // debug_.printf("[DEBUG] modbus_rtu_slave[%d] Timeout. framebuffer clear.\r\n", adr_);
#endif
    rx_buff_.clear();
  }

  idle_timer_.reset();
  rx_buff_.insert(rx_buff_.end(), src, src + size);

  // 頭出し
  while( rx_buff_.size() > 1 && rx_buff_[0] != adr_ )
    rx_buff_.erase(rx_buff_.begin());

  // 最低フレームサイズ(adr + cmd + crc)より少なければ次
  if(rx_buff_.size() < 4 )
    return;

  if(
    readcoilstatus_()
    || readinputstatus_()
    || readholdingregister_()
    || readinputregister_()
    || forcesinglecoil_()
    // || prisetsigleregister_()
    // || diagnostics_()
    // || fetchcommeventcounter_()
    // || fetchcommeventlog_()
    // || forcemultiplecoils_()
    // || presetmultipleregisters_()
    // || reportslaveid_()
  ){
    rx_buff_.clear();
  }
}

/**
 * @brief アイドル処理
 */
void modbus_rtu_slave::idle(std::vector<uint8_t>& tx_buff)
{
  if(idle_timer_.read_ms() >= idle_limit_ && !tx_buff_.empty() ){
    tx_buff.insert(tx_buff.end(), tx_buff_.begin(), tx_buff_.end());
    tx_buff_.clear();
  }
}

/**
 * @breaf 例外応答の生成
 */
void modbus_rtu_slave::exceptionresponse(std::vector<uint8_t>& dst, uint8_t adr, uint8_t cmd, uint8_t code)
{
  uint8_t except[5] = {
    adr,
    (uint8_t)(0x80 | cmd),
    code,
    0x00,0x00
  };

  uint16_t crc = crc16(except, 3);
  except[3] = (0xff & crc);
  except[4] = (crc >> 8) & 0xff;
  dst.insert(dst.end(), except, except + 5);
}

/**
 * @brief readcoilstatus応答(0x01)
 */
bool modbus_rtu_slave::readcoilstatus_(void)
{
  if(rx_buff_[1] != 0x01 ) return false;
  if(rx_buff_.size() < 8 ) return false;

  const uint16_t crc_src = rx_buff_[6] | (rx_buff_[7] << 8);
  const uint16_t crc_calc = crc16(&rx_buff_[0], 6);

  if(crc_src != crc_calc) {
#ifndef NDEBUG
    debug_.printf("[DEBUG] modbus_rtu_slave[%d] readcoilstatus_(): crc error. src = %04xh calc = %04xh(%d)\r\n", adr_,crc_src, crc_calc);
#endif
    return true;
  }
  const uint16_t start_adr = (rx_buff_[2] << 8) | rx_buff_[3];
  const uint16_t reg_cnt = (rx_buff_[4] << 8) | rx_buff_[5];

  readcoilstatus(tx_buff_, start_adr, reg_cnt);

#ifndef NDEBUG
  debug_.printf("[DEBUG] modbus_rtu_slave[%d] readcoilstatus_(): complate.\r\n", adr_);
#endif

  return true;
}

/**
 * @brief readcoilstatus応答(0x01)
 */
void modbus_rtu_slave::readcoilstatus(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt)
{
  //例外応答を返す
  exceptionresponse(dst, adr_, 0x01, 0x01);
}

/**
 * @brief readinputstatus応答(0x02)
 */
bool modbus_rtu_slave::readinputstatus_(void)
{
  if(rx_buff_[1] != 0x02 ) return false;
  if(rx_buff_.size() < 8 ) return false;

  const uint16_t crc_src = rx_buff_[6] | (rx_buff_[7] << 8);
  const uint16_t crc_calc = crc16(&rx_buff_[0], 6);

  if(crc_src != crc_calc) return true;

  const uint16_t start_adr = (rx_buff_[2] << 8) | rx_buff_[3];
  const uint16_t reg_cnt = (rx_buff_[4] << 8) | rx_buff_[5];

  readinputstatus(tx_buff_, start_adr, reg_cnt);
  return true;
}

/**
 * @brief readinputstatus応答(0x02)
 */
void modbus_rtu_slave::readinputstatus(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt)
{
  //例外応答を返す
  exceptionresponse(dst, adr_, 0x02, 0x01);
}

/**
 * @brief readholdingregister応答(0x03)
 */
bool modbus_rtu_slave::readholdingregister_(void)
{
  if(rx_buff_[1] != 0x03 ) return false;
  if(rx_buff_.size() < 8 ) return false;

  const uint16_t crc_src = rx_buff_[6] | (rx_buff_[7] << 8);
  const uint16_t crc_calc = crc16(&rx_buff_[0], 6);

  if(crc_src != crc_calc) return true;

  const uint16_t start_adr = (rx_buff_[2] << 8) | rx_buff_[3];
  const uint16_t reg_cnt = (rx_buff_[4] << 8) | rx_buff_[5];

  readholdingregister(tx_buff_, start_adr, reg_cnt);
  return true;
}

/**
 * @brief readholdingregister応答(0x03)
 */
void modbus_rtu_slave::readholdingregister(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt)
{
  //例外応答を返す
  exceptionresponse(dst, adr_, 0x03, 0x01);
}

/**
 * @brief readinputregister応答(0x04)
 */
bool modbus_rtu_slave::readinputregister_(void)
{
  if(rx_buff_[1] != 0x04 ) return false;
  if(rx_buff_.size() < 8 ) return false;

  const uint16_t crc_src = (rx_buff_[6] >> 8) | (rx_buff_[7] << 8);
  const uint16_t crc_calc = crc16(&rx_buff_[0], 6);

  if(crc_src != crc_calc) return true;

  const uint16_t start_adr = (rx_buff_[2] << 8) | rx_buff_[3];
  const uint16_t reg_cnt = (rx_buff_[4] << 8) | rx_buff_[5];

  readinputregister(tx_buff_, start_adr, reg_cnt);
  return true;
}

/**
 * @brief readinputregister応答(0x04)
 */
void modbus_rtu_slave::readinputregister(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t reg_cnt)
{

  //例外応答を返す
  exceptionresponse(dst, adr_, 0x04, 0x01);
}

/**
 * @brief forcesinglecoil応答(0x05)
 */
bool modbus_rtu_slave::forcesinglecoil_(void)
{
  if(rx_buff_[1] != 0x05 ) return false;
  if(rx_buff_.size() < 8 ) return false;

  const uint16_t crc_src = (rx_buff_[6] >> 8) | (rx_buff_[7] << 8);
  const uint16_t crc_calc = crc16(&rx_buff_[0], 6);

  if(crc_src != crc_calc) return true;

  const uint16_t start_adr = (rx_buff_[2] << 8) | rx_buff_[3];
  const uint16_t value = (rx_buff_[4] << 8) | rx_buff_[5];

  forcesinglecoil(tx_buff_, start_adr, value);
  return true;
}

/**
 * @brief forcesinglecoil応答(0x05)
 */
void modbus_rtu_slave::forcesinglecoil(std::vector<uint8_t>& dst, uint16_t start_adr, uint16_t value)
{
  //例外応答を返す
  exceptionresponse(dst, adr_, 0x05, 0x01);
}

} /* namespace */
