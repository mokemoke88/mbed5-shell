/**
 * @file vars.h
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-08
 * @par history
 * - 2016-11-08 16:55:37
 *  - First.
 */

#ifndef VARS_H
#define VARS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "mbed.h"
#include "seekers/mbed/rs485serial.hpp"

#define SELF_VERSION "1.0.0"

typedef Callback<void(void)> runtime_loop_t;

extern runtime_loop_t runtime_loop;

extern RawSerial pc;
extern seekers::RS485Serial uart;

extern int uart_baud_;
extern int uart_bits_;
extern SerialBase::Parity uart_parity_;
extern int uart_stop_bits_;

#endif /* VARS_H */
