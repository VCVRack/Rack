#include "integer.h"

DWORD get_fattime() {
  return    ((DWORD)(2011 - 1980) << 25)
      | ((DWORD)4 << 21)
      | ((DWORD)15 << 16)
      | ((DWORD)1 << 11)
      | ((DWORD)6 << 5)
      | ((DWORD)0 >> 1); 
}
