/**
 * @file modbus_rtu_master.cpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-05
 * @par history
 * - 2016-11-05 09:45:10
 *  - first.
 */

#include "mbed.h"
#include "modbus_rtu_master.hpp"

namespace seekers{

/**
 * @brief readcoilstatus要求フレームを生成
 */
void modbus::request_readcoilstatus(std::vector<uint8_t>& dst, uint8_t slave, uint16_t reg_adr, uint16_t reg_cnt)
{
  uint8_t data[8] = {
    slave, 0x01,
    (uint8_t)(reg_adr >> 8), (uint8_t)(reg_adr),
    (uint8_t)(reg_cnt >> 8), (uint8_t)(reg_cnt)
  };
  uint16_t crc = crc16_ibm(data, 6);
  data[6] = 0xFF & crc;
  data[7] = 0xFF & (crc >> 8);
  dst.insert(dst.end(), data, data + 8);
}


/**
 * @brief 応答ハンドラを設定
 */
void modbus_rtu_master::sethandler(response_handler_t handler, handler_type_t handler_type)
{
  switch(handler_type)
  {
  case READCOILSTATUS:
    readcoilstatus_handler_ = handler;
    break;
  case EXCEPTIONRESPONSE:
    exceptionresponse_handler_ = handler;
    break;
  }
}

/**
 * @brief readcoilstatus要求フレームを生成、応答待ち状態への遷移
 */
void modbus_rtu_master::request_readcoilstatus(std::vector<uint8_t>& dst, uint8_t slave, uint16_t reg_adr, uint16_t reg_cnt)
{
  modbus::request_readcoilstatus(dst, slave, reg_adr, reg_cnt);

  tgt_slave_ = slave;
  tgt_cmd_ = 0x01;
  stat_ = STAT_WAIT_FOR_REQUEST;
  response_timer_.start();
  response_timer_.reset();
}

/**
 * @brief アイドル処理
 */
void modbus_rtu_master::idle(std::vector<uint8_t>& tx_buff)
{
  if(stat_ != STAT_WAIT_FOR_REQUEST) return;

  // 応答待ちタイムアウトを図る
  if( response_timer_.read_ms() >= response_limit_ ){
    if(NULL != response_timeout_handler_)
      response_timeout_handler_(this, tgt_slave_, tgt_cmd_);
#ifndef NDEBUG
    debug_.printf("[DEBUG] modubs_rtu_master::idle() response_timeout.\r\n");
#endif
    rx_buff_.clear();
    stat_ = STAT_HALT;
  }
}

/**
 * @brief 受信処理
 */
void modbus_rtu_master::recieve(std::vector<uint8_t>& tx_buff, const uint8_t* src, size_t size)
{
  if(idle_timer_.read_ms() >= idle_limit_ ){
    rx_buff_.clear();
  }
  idle_timer_.reset();
  rx_buff_.insert(rx_buff_.end(), src, src + size);

  if(stat_ != STAT_WAIT_FOR_REQUEST)
    return;

  // 応答待ちタイムアウトを図る
  if( response_timer_.read_ms() >= response_limit_ ){
    if(NULL != response_timeout_handler_)
      response_timeout_handler_(this, tgt_slave_, tgt_cmd_);
#ifndef NDEBUG
    debug_.printf("[DEBUG] modubs_rtu_master::recieve() response_timeout.\r\n");
#endif
    rx_buff_.clear();
    stat_ = STAT_HALT;
  }

  // 頭出し
  while( rx_buff_.size() > 1 && rx_buff_[0] != tgt_slave_ )
    rx_buff_.erase(rx_buff_.begin());

  // 最低フレームサイズ(adr + cmd + crc)より少なければ次
  if(rx_buff_.size() < 4 )
    return;

  bool request_result = false;
  switch(tgt_cmd_){
  case 0x01:
    request_result = readcoilstatus_() | exceptionresponse_();
    break;
    /*
  case 0x02:
    result_request = readholdingregister_() | exceptionresponse_();
    break;
  case 0x03:
    request_result = readinputregister_() | exceptionresponse_();
    break;
  case 0x04:
    request_result = forcesinglecoil_() | exceptionresponse_();
    break;
  case 0x05:
    request_result = presetsingleregister_() | exceptionresponse_();
    break;
  case 0x06:
    request_result = diagnostics_() | exceptionresponse_();
    break;
  case 0x07:
    request_result = fetchcommeventcounter_() | exceptionresponse_();
    break;
  case 0x08:
    request_result = fetchcommeventlog_() | exceptionresponse_();
    break;
  case 0x09:
    request_result = forcemultiplecoils_() | exceptionresponse_();
    break;
  case 0x10:
    request_result = presetmultipleregisters_() | exceptionresponse_();
    break;
  case 0x11:
    request_result = reportslaveid_() | exceptionresponse_();
    break;
    */
  }
  if(request_result){
    rx_buff_.clear();
    stat_ = STAT_HALT;
  }
}

/**
 * @brief 例外応答を対応
 */
bool modbus_rtu_master::exceptionresponse_(void)
{
  if(rx_buff_[1] != (0x80 | tgt_cmd_) ) return false;
  if(rx_buff_.size() < 5 ) return false;

  const uint16_t crc_src = rx_buff_[3] | (rx_buff_[4] << 8);
  const uint16_t crc_calc = crc16(&rx_buff_[0], 3);

  if(crc_src != crc_calc) {
#ifndef NDEBUG
    debug_.printf("[DEBUG] modbus_rtu_master exceptionresponse_(): crc error. src = %04xh calc = %04xh(%d)\r\n",crc_src, crc_calc);
#endif
    return true;
  }

  if(NULL != exceptionresponse_handler_)
    exceptionresponse_handler_(this, &rx_buff_[0], 3 + 2);

#ifndef NDEBUG
  debug_.printf("[DEBUG] modbus_rtu_master exceptionresponse_(): complate.\r\n");
#endif

  return true;
}

/**
 * @brief readcoilstatus応答を対応
 */
bool modbus_rtu_master::readcoilstatus_(void)
{
  if(rx_buff_[1] != 0x01) return false;
  if(rx_buff_.size() < 3 ) return false;

  const size_t data_byte = rx_buff_[2];
  if(rx_buff_.size() < (3 + 2 + data_byte)) return false;

  const uint16_t crc_src = rx_buff_[3 + data_byte] | (rx_buff_[3 + data_byte + 1] << 8);
  const uint16_t crc_calc = crc16(&rx_buff_[0], 3 + data_byte);

  if(crc_src != crc_calc) {
#ifndef NDEBUG
    debug_.printf("[DEBUG] modbus_rtu_master readcoilstatus_(): crc error. src = %04xh calc = %04xh(%d)\r\n",crc_src, crc_calc);
#endif
    return true;
  }

  if(NULL != readcoilstatus_handler_)
    readcoilstatus_handler_(this, &rx_buff_[0], 3 + data_byte + 2);

#ifndef NDEBUG
  debug_.printf("[DEBUG] modbus_rtu_master readcoilstatus_(): complate.\r\n");
#endif

  return true;
}

} /* namespace */
