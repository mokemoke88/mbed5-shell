/**
 * @file seekers/mbed/sn74xx595.hpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-10
 * @par history
 * - 2016-11-10 15:09:54
 *  - First.
 */

#ifndef SEEKERS_MBED_SN74XX595_HPP
#define SEEKERS_MBED_SN74XX595_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "mbed.h"
#include "rtos.h"

namespace seekers{
/**
 * @brief 8bitシフトレジスタ SN74xx595
 */
template <typename T>
class sn74xx595{
private:
  DigitalOut sck_;
  DigitalOut rck_;
  DigitalOut si_;
  Semaphore one_slot_;
  T value_;
  Thread t_;

private:
  static void out_(const void* ptr)
  {
    sn74xx595* self = (sn74xx595*)ptr;
    self->one_slot_.wait();
    self->rck_ = 0;
    self->sck_ = 0;
    for(size_t ii = 0; ii < (sizeof(T) * 8); ++ii)
    {
      self->si_ = ((self->value_ >> ( (sizeof(T) * 8) - 1 - ii)) & 0x01 );
      self->sck_ = 1;
      self->sck_ = 0;
    }
    self->rck_ = 1;
    self->rck_ = 0;
    self->one_slot_.release();
  }

public:
  /** constractor
   * use 3pins SCK, RCK, SI for output.
   * @param sck PinName of SCK
   * @param rck PinName of RCK
   * @param si PinName of SI
   */
  sn74xx595(PinName sck, PinName rck, PinName si) :
    sck_(sck),
    rck_(rck),
    si_(si),
    one_slot_(1),
    value_(0)
  {
    sck_ = 0;
    rck_ = 0;
    si_ = 0;
  }

  /** output bitdata
   * output bit data. A1 ... A8 = LSB ... MSB
   * @param src output data. IC A1 ... A8 = LSB ... MSB
   * @return none
   */
  void out(T src)
  {
    one_slot_.wait();
    value_ = src;
    t_.start(callback(sn74xx595<T>::out_, (void*)this));
    one_slot_.release();
  }
};

}

#endif /* SEEKERS_MBED_SN74XX595_HPP */
