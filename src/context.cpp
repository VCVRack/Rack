#include "context.hpp"


namespace rack {


static Context c;


Context *context() {
	return &c;
}


} // namespace rack
