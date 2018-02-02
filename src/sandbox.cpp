#include "sandbox.hpp"
#include "util.hpp"
#include "asset.hpp"
#include <vector>

namespace rack {
#if defined(ARCH_MAC)
#include <sandbox.h>
#include <stdlib.h>
#include <sys/syslimits.h>
extern "C" {

int sandbox_init_with_parameters(const char *profile, uint64_t flags, const char *const parameters[], char **errorbuf);
// Possible values for 'flags':
#define SANDBOX_STRING  0x0000
#define SANDBOX_NAMED   0x0001
#define SANDBOX_BUILTIN 0x0002
#define SANDBOX_FILE    0x0003

}

struct SandboxParams {
  void* buf;
  size_t count;
  size_t size;
};

bool sandboxInit() {

  /*
  char* params = sandbox_create_params();
  if (!params)
    return false;

  sandbox_set_param(params, "rackGlobal", assetGlobal("").c_str());
  sandbox_set_param(params, "rackLocal", assetLocal("").c_str());
  */
  std::string profilePath = assetGlobal("Rack.sb").c_str();
  FILE *file = fopen(profilePath.c_str(), "rb");
  if(!file) {
    info("Couldn't read sandbox profile");
    return false;
  }
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);  //same as rewind(f);

  char *profileStr = (char*)malloc(fsize + 1);
  fread(profileStr, fsize, 1, file);
  fclose(file);
  profileStr[fsize] = 0;

  std::vector<const char *> params;
  char resolved_path[PATH_MAX];

  params.push_back("rackGlobal");
  realpath(assetGlobal("").c_str(), resolved_path);
  params.push_back(resolved_path);

  params.push_back("rackLocal");
  realpath(assetLocal("").c_str(), resolved_path);
  params.push_back(resolved_path);

  // The parameters array is null terminated.
  params.push_back(nullptr);

  char* error_buff = nullptr;
  int error = sandbox_init_with_parameters(profileStr, SANDBOX_STRING, params.data(), &error_buff);
  bool success = (error == 0 && error_buff == NULL);
  if(!success) {
    info("Sandbox initialization error (%d): %s", error, error_buff);
  } else {
    info("Sandbox initialized!");
  }
  sandbox_free_error(error_buff);

  FILE *file2 = fopen("/Users/jon/Documents/mg.txt", "rb");
  if(!file2) {
    info("Couldn't read private file");
  }

  return success;
}


#else
	bool sandboxInit() {
    return false;
  }
#endif
}
