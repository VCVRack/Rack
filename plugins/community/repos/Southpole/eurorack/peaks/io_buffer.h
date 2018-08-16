// Copyright 2015 Olivier Gillet.
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
// I/O Buffering.

#ifndef PEAKS_IO_BUFFER_H_
#define PEAKS_IO_BUFFER_H_

#include "stmlib/stmlib.h"
#include "peaks/gate_processor.h"

#include <algorithm>

namespace peaks {

const size_t kNumBlocks = 2;
const size_t kNumChannels = 2;
const size_t kBlockSize = 4;

class IOBuffer {
 public:
  struct Block {
    GateFlags input[kNumChannels][kBlockSize];
    uint16_t output[kNumChannels][kBlockSize];
  };
  
  struct Slice {
    Block* block;
    size_t frame_index;
  };

  typedef void ProcessFn(Block* block, size_t size);

  IOBuffer() { }
  ~IOBuffer() { }
  
  void Init() {
    io_block_ = 0;
    render_block_ = kNumBlocks / 2;
    io_frame_ = 0;
  }
  
  inline void Process(ProcessFn* fn) {
    while (render_block_ != io_block_) {
      (*fn)(&block_[render_block_], kBlockSize);
      render_block_ = (render_block_ + 1) % kNumBlocks;
    }
  }
  
  inline Slice NextSlice(size_t size) {
    Slice s;
    s.block = &block_[io_block_];
    s.frame_index = io_frame_;
    io_frame_ += size;
    if (io_frame_ >= kBlockSize) {
      io_frame_ -= kBlockSize;
      io_block_ = (io_block_ + 1) % kNumBlocks;
    }
    return s;
  }
  
  inline bool new_block() const {
    return io_frame_ == 0;
  }
  
 private:
  Block block_[kNumBlocks];
  
  size_t io_frame_;
  volatile size_t io_block_;
  volatile size_t render_block_;
  
  DISALLOW_COPY_AND_ASSIGN(IOBuffer);
};

}  // namespace peaks

#endif  // PEAKS_IO_BUFFER_H_
