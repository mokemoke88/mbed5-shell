/**
 * @file mbed/rs485serial.hpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-04
 * @par history
 * - 2016-11-04 07:57:06
 *  - First.
 */

#ifndef SEEKERS_MBED_RS485SERIAL_HPP
#define SEEKERS_MBED_RS485SERIAL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(__MBED__)

#include "mbed.h"
#include "CircularBuffer.h"

namespace seekers{

/**
 * @brief RS485 半二重制御付きシリアル
 */
class RS485Serial : public RawSerial
{
private:
  static const int BUFSIZE = 64;
  static const int STDBUFSIZE = 64; // printf使用時のバッファサイズ(スタック消費量)
  DigitalOut we_;
  Timeout we_timer_;

  CircularBuffer<uint8_t, BUFSIZE> rx_buff_; // 受信バッファ

  bool auto_dessert_;

  int baud_;
  int bit_length_;
  double we_time_;

  static void we_timer_handler_(RS485Serial*);
  static void tx_handler_(RS485Serial*);
  static void rx_handler_(RS485Serial*);

  int getc_(void);

  void update_we_time_(void);

  // method disable.
  void attach(Callback<void()>, IrqType);

public:
  RS485Serial(PinName tx = p9, PinName rx = p10, PinName we = p8);

  ~RS485Serial()
  {}

  int readable(void);
  void baud(int baudrate);
  void format(int bits = 8, Parity parity=SerialBase::None, int stop_bits=1);
  int getc(void);
  void putc(int c);
  int printf(const char* format, ...);

  void we_assert(bool auto_dessert = true);
  void we_dessert(void);
};


inline void RS485Serial::update_we_time_(void)
{
  we_time_ = ((double)bit_length_ * 1.0) / (double)baud_;
}

inline int RS485Serial::readable(void)
{
  return ( (!rx_buff_.empty()) ? 1 : 0 );
}

inline void RS485Serial::baud(int baudrate)
{
  RawSerial::baud(baudrate);
  baud_ = baudrate;
  update_we_time_();
}

inline void RS485Serial::format(int bits, Parity parity, int stop)
{
  RawSerial::format(bits, parity, stop);
  bit_length_ = 1.0
    + bits
    + (( parity != SerialBase::None ) ? 1.0 : 0.0 )
    + stop;
  update_we_time_();
}

inline int RS485Serial::getc(void)
{
    if(rx_buff_.empty()) return -1;

    uint8_t b = 0;
    rx_buff_.pop(b);
    return b;
}

inline void RS485Serial::putc(int c)
{
  we_assert();
  RawSerial::putc(c);
}

inline void RS485Serial::we_assert(bool auto_dessert)
{
  we_timer_.detach(); // アサート後に割り込みでデサートされる可能性の排除
  if(we_ == 0 ){
    we_ = 1;
  }
  auto_dessert_ = auto_dessert;
}

inline void RS485Serial::we_dessert(void)
{
  we_ = 0;
  auto_dessert_ = true;
}

inline int RS485Serial::getc_(void)
{
  return RawSerial::getc();
}

} /* namespace */

#endif /* __MBED__ */

#endif /* SEEKERS_MBED_RS485SERIAL_HPP */
