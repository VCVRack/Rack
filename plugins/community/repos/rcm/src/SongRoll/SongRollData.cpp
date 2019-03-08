#include "../../include/SongRoll/SongRollData.hpp"

namespace rack_plugin_rcm {

using namespace SongRoll;

Section::Section() {
  channels.resize(8);
}

SongRollData::SongRollData() {
  sections.resize(1);
}

void SongRollData::reset() {

}

} // namespace rack_plugin_rcm
