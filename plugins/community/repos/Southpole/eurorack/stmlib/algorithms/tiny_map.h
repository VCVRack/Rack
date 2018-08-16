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
// A rather inefficient (lookup and insertion in O(n)) map. Useful for storing
// very small mappings (say about 16 values).

#ifndef STMLIB_ALGORITHMS_TINY_MAP_H_
#define STMLIB_ALGORITHMS_TINY_MAP_H_

#include "stmlib/stmlib.h"

#include <cstring>

namespace stmlib {

template<
    typename Key,
    typename Value,
    uint8_t capacity,
    uint8_t EMPTY = 0xff>
class TinyMap {
 public:
  struct Entry {
    Key key;
    Value value;
  };
  
  TinyMap() { }
  ~TinyMap() { }
    
  void Init() {
    Clear();
  }
  
  void Put(Key key, Value value) {
    Entry* entry = Search(key);
    if (entry == NULL) {
      entry = SearchFreeSlot();
    }
    if (entry == NULL) {
      entry = &map_[0];
    }
    entry->key = key;
    entry->value = value;
    // Speed up retrieval for next query, in the very common case that when
    // element is inserted, the next query is for this same element.
    if (value != EMPTY) {
      recent_search_ = entry;
    } else {
      recent_delete_ = entry;
    }
  }
  
  const Entry* Find(uint8_t key) {
    return Search(key);
  }

  void Clear() {
    memset(map_, EMPTY, capacity * sizeof(Entry));
    recent_search_ = recent_delete_ = &map_[0];
  }

 private:
  Entry* Search(Key key) {
    if (recent_search_->key == key) {
      return recent_search_;
    }
    for (uint8_t i = 0; i < capacity; ++i) {
      if (map_[i].key == key) {
        recent_search_ = &map_[i];
        return recent_search_;
      }
    }
    return NULL;
  }
  
  Entry* SearchFreeSlot() {
    if (recent_delete_->value == EMPTY) {
      return recent_delete_;
    }
    for (uint8_t i = 0; i < capacity; ++i) {
      if (map_[i].value == EMPTY) {
        recent_delete_ = &map_[i];
        return recent_delete_;
      }
    }
    return NULL;
  }
   
  Entry map_[capacity];
  Entry* recent_search_;
  Entry* recent_delete_;
  DISALLOW_COPY_AND_ASSIGN(TinyMap);
};

}  // namespace stmlib

#endif  // STMLIB_ALGORITHMS_TINY_MAP_H_
