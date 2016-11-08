/**
 * @file vars.cpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-08
 * @par history
 * - 2016-11-08 16:56:27
 *  - first.
 */

#include "vars.h"

runtime_loop_t runtime_loop = NULL;
RawSerial pc(USBTX, USBRX);

seekers::RS485Serial uart(p9,p10,p8); //seekers::RS485Serial uart(p9,p10,p8);

int uart_baud_ = 9600;
int uart_bits_ = 8;
SerialBase::Parity uart_parity_ = Serial::None;
int uart_stop_bits_ = 1;
