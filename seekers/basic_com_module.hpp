/**
 * @file basic_comm_module.hpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-07
 * @par history
 * - 2016-11-07 20:05:56
 *  - First.
 */

#ifndef SEEKERS_BASIC_COMM_MODULE_HPP
#define SEEKERS_BASIC_COMM_MODULE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace seekers{


typedef std::vector<uint8_t>::iterator buff_iter_t;


/**
 * @brief com_module interface.
 */
class basic_com_module{
public:
  virtual void recieve(std::vector<uint8_t>& tx_buf, const uint8_t* src, size_t size) = 0;
  virtual void idle(std::vector<uint8_t>& tx_buf) = 0;

  virtual ~basic_com_module(){}
};


} /* namespace */


#endif /* SEEKERS_BASIC_COMM_MODULE_HPP */
