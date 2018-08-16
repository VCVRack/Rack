// Copyright 2012 Olivier Gillet.
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
// Stream buffer for serialization.

#ifndef STMLIB_UTILS_STREAM_BUFFER_H_
#define STMLIB_UTILS_STREAM_BUFFER_H_

#include "stmlib/stmlib.h"

#include <cstring>
#include <algorithm>

namespace stmlib {

template<size_t buffer_size>
class StreamBuffer {
 public:
  StreamBuffer() { Clear(); }
  
  void Clear() {
    ptr_ = 0;
    std::fill(&buffer_[0], &buffer_[buffer_size], 0);
  }
  
  inline size_t position() const {
    return ptr_;
  }

  inline const uint8_t* bytes() const {
    return buffer_;
  }

  inline uint8_t* mutable_bytes() {
    return buffer_;
  }

  void Write(const void* data, size_t size) {
    if (ptr_ + size > buffer_size) {
      return;
    }
    memcpy(&buffer_[ptr_], data, size);
    ptr_ += size;
  }
  
  template<typename T>
  void Write(const T& value) {
    Write(&value, sizeof(T));
  }

  template<typename T>
  void Read(T* value) {
    if (ptr_ + sizeof(T) > buffer_size) {
      return;
    }
    memcpy((void*)value, &buffer_[ptr_], sizeof(T));
    ptr_ += sizeof(T);
  }
  
  inline void Seek(size_t position) {
    ptr_ = position;
  }
  
  inline void Rewind() { Seek(0); }

 private:
  uint8_t buffer_[buffer_size];
  size_t ptr_;

  DISALLOW_COPY_AND_ASSIGN(StreamBuffer);
};

}  // namespace stmlib

#endif   // STMLIB_UTILS_STREAM_BUFFER_H_
