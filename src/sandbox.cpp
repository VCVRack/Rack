#include "sandbox.hpp"
#include "util.hpp"
namespace rack {
#if defined(ARCH_MAC)
#include <sandbox.h>

bool sandboxInit() {
  char* error_buff = NULL;
  //int error = sandbox_init(final_sandbox_profile_str.c_str(), 0, &error_buff);
  int error = sandbox_init(kSBXProfileNoInternet, SANDBOX_NAMED, &error_buff);
  bool success = (error == 0 && error_buff == NULL);
  if(!success) {
    info("Sandbox initialization error (%d): %s", error, error_buff);
  } else {
    info("Sandbox initialized!");
  }
  sandbox_free_error(error_buff);

  return success;
}


#else
	bool sandboxInit() {
    return false;
  }
#endif
}
