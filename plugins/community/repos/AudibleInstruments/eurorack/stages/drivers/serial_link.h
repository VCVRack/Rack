// Copyright 2017 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// UART driver for agreeable conversations with the neighbors.

#ifndef STAGES_DRIVERS_SERIAL_LINK_H_
#define STAGES_DRIVERS_SERIAL_LINK_H_

#include "stmlib/stmlib.h"

#include <stm32f37x_conf.h>

namespace stages {

enum SerialLinkDirection {
  SERIAL_LINK_DIRECTION_LEFT,
  SERIAL_LINK_DIRECTION_RIGHT
};

class SerialLink {
 public:
  SerialLink() { }
  ~SerialLink() { }
  
  void Init(
      SerialLinkDirection direction,
      uint32_t baud_rate,
      uint8_t* rx_buffer,
      size_t rx_block_size);
  
  void Transmit(const void* buffer, size_t size);
  
  template<typename T>
  void Transmit(const T& t) {
    Transmit(&t, sizeof(T));
  }
  
  bool tx_complete();
  
  // For polled RX: call Receive(destination, size);
  // Then when rx_complete() is true, _destination has been filled with
  // _size bytes.
  void Receive(void* buffer, size_t size);
  bool rx_complete();
  
  // For continuous RX: returns NULL if no data is ready, or a pointer if
  // a buffer has been received.
  const uint8_t* available_rx_buffer();
  
  template<typename T>
  inline const T* available_rx_buffer() {
    return static_cast<const T*>(
        static_cast<const void*>(available_rx_buffer()));
  }
  
 private:
  SerialLinkDirection direction_;
  size_t rx_block_size_;
  uint8_t* rx_buffer_;
  
  DISALLOW_COPY_AND_ASSIGN(SerialLink);
};

}  // namespace stages

#endif  // STAGES_DRIVERS_SERIAL_LINK_H_
