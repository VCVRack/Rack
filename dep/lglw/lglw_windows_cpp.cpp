/* ----
 * ---- file   : lglw_windows_cpp.cpp
 * ---- author : bsp
 * ---- legal  : Distributed under terms of the MIT LICENSE (MIT).
 * ----
 * ---- Permission is hereby granted, free of charge, to any person obtaining a copy
 * ---- of this software and associated documentation files (the "Software"), to deal
 * ---- in the Software without restriction, including without limitation the rights
 * ---- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * ---- copies of the Software, and to permit persons to whom the Software is
 * ---- furnished to do so, subject to the following conditions:
 * ----
 * ---- The above copyright notice and this permission notice shall be included in
 * ---- all copies or substantial portions of the Software.
 * ----
 * ---- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * ---- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * ---- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * ---- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * ---- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * ---- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * ---- THE SOFTWARE.
 * ----
 * ---- info   : This is part of the "lglw" package.
 * ----
 * ---- created: 09Aug2018
 * ---- changed: 
 * ----
 * ----
 */

#include "lglw.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <windowsx.h>
#include <initguid.h>  // for touch keyboard
#include <Objbase.h>  // for touch keyboard
#include <Shobjidl.h>  // for IFrameworkInputPane


// ---------------------------------------------------------------------------- lglw_int_touchkeyboard_toggle
// 4ce576fa-83dc-4F88-951c-9d0782b4e376
DEFINE_GUID(CLSID_UIHostNoLaunch,
    0x4CE576FA, 0x83DC, 0x4f88, 0x95, 0x1C, 0x9D, 0x07, 0x82, 0xB4, 0xE3, 0x76);

// 37c994e7_432b_4834_a2f7_dce1f13b834b
DEFINE_GUID(IID_ITipInvocation,
    0x37c994e7, 0x432b, 0x4834, 0xa2, 0xf7, 0xdc, 0xe1, 0xf1, 0x3b, 0x83, 0x4b);

struct ITipInvocation : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Toggle(HWND wnd) = 0;
};

extern "C" lglw_bool_t lglw_int_touchkeyboard_toggle(void) {
   // by "torvin", <https://stackoverflow.com/questions/38774139/show-touch-keyboard-tabtip-exe-in-windows-10-anniversary-edition>
   HRESULT hr;
   lglw_bool_t r = LGLW_FALSE;
   ITipInvocation *tip;

   hr = CoInitialize(0);
   hr = CoCreateInstance(CLSID_UIHostNoLaunch, 0, CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_ITipInvocation, (void**)&tip);

   if(NULL != tip)
   {
      tip->Toggle(GetDesktopWindow());
      tip->Release();
      r = LGLW_TRUE;
   }

   return r;
}
