/**
 * @file mbed/rs485serial.cpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-04
 * @par history
 * - 2016-11-04 08:14:42
 *  - first.
 */

#if defined(__MBED__)

#include <cstdarg>
#include "mbed.h"
#include "rs485serial.hpp"


namespace seekers{


/**
 * @brief コンストラクタ
 */
RS485Serial::RS485Serial(PinName tx, PinName rx, PinName we) :
  RawSerial(tx, rx),
  we_(we),
  auto_dessert_(true),
  baud_(9600),
  bit_length_(10),
  we_time_( 0.0 )
{
  we_ = 0;
  RawSerial::attach( callback(this, &RS485Serial::tx_handler_), Serial::TxIrq);
  RawSerial::attach( callback(this, &RS485Serial::rx_handler_), Serial::RxIrq);
  update_we_time_();
}

/**
 * @brief 書式付き出力
 */
int RS485Serial::printf(const char* format, ...)
{
  va_list arg;
  char buff[STDBUFSIZE];
  va_start(arg, format);
  int n = vsprintf(buff, format, arg);
  va_end(arg);
  if(0 >= n) return n;

  for(int ii = 0; ii < n; ++ii)
    putc(buff[ii]);

  return n;
}

/**
 * @brief 送信割り込みハンドラ
 * 送信割り込み後、タイマー割り込みを起動
 */
void RS485Serial::tx_handler_(RS485Serial* self)
{
  self->we_timer_.detach();
  if(self->auto_dessert_){
    self->we_timer_.attach( callback(self, &RS485Serial::we_timer_handler_), self->we_time_ );
  }
}

/**
 * @brief 受信割り込みハンドラ
 * 受信バッファにデータを取り込み
 */
void RS485Serial::rx_handler_(RS485Serial* self)
{
  int c = self->getc_();
  if(c >= 0 && !self->rx_buff_.full())
    self->rx_buff_.push(c);
}

/**
 * @brief weデサート用タイマハンドラ
 */
void RS485Serial::we_timer_handler_(RS485Serial* self)
{
  self->we_dessert();
}

} /* namespace */

#endif /* __MBED__ */
