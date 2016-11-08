/**
 * @file utils.hpp
 * @brief ユーティリティ関数群
 * @author kshibata@seekers.jp
 * @date 2016-11-04
 * @par history
 * - 2016-11-04 10:58:27
 *  - First.
 */

#ifndef SEEKERS_UTILS_HPP
#define SEEKERS_UTILS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace seekers{

typedef unsigned short int uint16_t;

/**
 * @brief crc16計算
 */
template <int POLY>
uint16_t crc16_base(const uint8_t* data, size_t size)
{
  static const uint16_t CRC16POLY = POLY;
  uint16_t crc = 0;
  crc = ~crc;
  for(size_t ii = 0; ii < size; ++ii){
    crc ^= *(data + ii);
    for(int jj = 0; jj < 8; ++jj){
      if(crc & 1)
        crc = (crc >> 1) ^ CRC16POLY;
      else
        crc >>= 1;
    }
  }
  return crc;
}

inline uint16_t crc16_ibm(const uint8_t* data, size_t size)
{
  return crc16_base<0xa001>(data,size);
}

inline uint16_t crc16_ansi(const uint8_t* data, size_t size)
{
  return crc16_base<0x8401>(data, size);
}

inline uint16_t crc16_ccitt(const uint8_t* data, size_t size)
{
  return crc16_base<0x8408>(data, size);
}

/**
 * @brief bcc計算
 */
inline uint8_t bcc(const uint8_t* data, size_t size)
{
  uint8_t bcc = 0x00;
  for(size_t ii = 0; ii < size; ++ii)
    bcc ^= *(data + ii);
  return bcc;
}

/**
 * @brief ascii bcd -> hexへ変換
 */
inline int asciibcd2int(const uint8_t* data, size_t size)
{
  int d = 0;
  int sign = 1;
  for(size_t ii = 0; ii < size; ++ii){
    if( d > 10 ) d *= 10;
    if( d == 0 && '-' == *(data + ii) ){
      sign *= -1;
      continue;
    }
    int t = *(data + ii) - 0x30;
    d += (0 > t || 9 < t) ? 0 : t;
  }
  return d * sign;
}

inline void fmt__(char* dst, int n)
{
  sprintf(dst, "%%%dd", n);
}

inline void fmtzero__(char* dst, int n)
{
  sprintf(dst, "%%0%dd", n);
}

/**
 * @brief T桁ascii bcd出力
 */
template <int T>
uint8_t* int2asciibcd(uint8_t* dst, int value)
{
  char fmt[8];
  fmt__(fmt, T);
  return (uint8_t*)sprintf((char*)dst, fmt, value);
}

/**
 * @brief T桁ascii bcd出力(0パディング版)
 */
template <int T>
uint8_t* int2asciibcd_zero(uint8_t* dst, int value)
{
  char fmt[8];
  fmtzero__(fmt, T);
  return (uint8_t*)sprintf((char*)dst, fmt, value);
}

} /* namespace */

#endif /* SEEKERS_UTILS_HPP */
