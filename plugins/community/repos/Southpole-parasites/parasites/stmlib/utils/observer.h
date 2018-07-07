// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech <matthias.puech@gmail.com>
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
// Observer pattern.

class Observable0 {
private:
  typedef void (*ObserverFct)();
  ObserverFct observer_;

public:
  void notify() {
    observer_();
  }
  void set_observer(ObserverFct observer) { observer_ = observer; }
};

template <typename Data>
class Observable1 {
private:
  typedef void (*ObserverFct)(Data);
  ObserverFct observer_;

public:
  void notify(Data data) {
    observer_(data);
  }
  void set_observer(ObserverFct observer) { observer_ = observer; }
};

template <typename Data1, typename Data2>
class Observable2 {
private:
  typedef void (*ObserverFct)(Data1, Data2);
  ObserverFct observer_;

public:
  void notify(Data1 data1, Data2 data2) {
    observer_(data1, data2);
  }
  void set_observer(ObserverFct observer) { observer_ = observer; }
};
