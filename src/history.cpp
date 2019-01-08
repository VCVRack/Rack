#include "history.hpp"


namespace rack {
namespace history {


void State::push(Action *action) {

}

void State::undo() {
	DEBUG("undo");
}

void State::redo() {
	DEBUG("redo");
}


} // namespace history
} // namespace rack
