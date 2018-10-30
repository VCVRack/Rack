#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(SubmarineFree);

#ifdef USE_VST2
#define plugin "SubmarineFree"
#endif // USE_VST2

#include "ComponentLibrary/components.hpp"

struct SubHelper {
	static std::shared_ptr<SVG> LoadPanel(const char/*Plugin*/ *_plugin, const char *str, int num) {
		char workingSpace[100];
		snprintf(workingSpace, 100, "res/%s%02d.svg", str, num);
		return SVG::load(assetPlugin(_plugin, workingSpace));
	}
};
