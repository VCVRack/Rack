// Copyright 2013 Olivier Gillet.
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
// Responsible for flash memory storage.

#ifndef YARNS_STORAGE_MANAGER_H_
#define YARNS_STORAGE_MANAGER_H_

#include <algorithm>

#include "stmlib/stmlib.h"

#include "stmlib/utils/stream_buffer.h"
#include "stmlib/system/storage.h"

namespace yarns {

class StorageManager {
 public:
  StorageManager() { }
  ~StorageManager() { }
  
  void SaveMulti(uint8_t slot);
  bool LoadMulti(uint8_t slot);
  void SaveCalibration();
  bool LoadCalibration();
  void SysExSendMulti();
  
  void AppendData(const uint8_t* data, size_t size, bool rewind) {
    if (rewind) {
      stream_buffer_.Rewind();
    }
    stream_buffer_.Write(data, size);
  }
  
  void DeserializeMulti();

 private:
  stmlib::StreamBuffer<1024> stream_buffer_;
  stmlib::Storage<0x8020000, 9> storage_;
  
  DISALLOW_COPY_AND_ASSIGN(StorageManager);
};

extern StorageManager storage_manager;

}  // namespace yarns

#endif // YARNS_STORAGE_MANAGER_H_
