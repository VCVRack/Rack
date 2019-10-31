#include <app/common.hpp>


namespace rack {
namespace app {


const std::string APP_NAME = "VCV Rack";
const std::string APP_VERSION = TOSTRING(VERSION);
#if defined ARCH_WIN
	const std::string APP_ARCH = "win";
#elif ARCH_MAC
	const std::string APP_ARCH = "mac";
#elif defined ARCH_LIN
	const std::string APP_ARCH = "lin";
#endif

const std::string ABI_VERSION = "2";

const std::string API_URL = "https://api.vcvrack.com";
const std::string API_VERSION = "2";


} // namespace app
} // namespace rack
