#include "TSScopeBase.hpp"


// List of what global effects to offer on each waveform.
const GlobalEffect* SCOPE_GLOBAL_EFFECTS[TROWA_NUM_GLOBAL_EFFECTS] = {
	new GlobalEffect("SRC-OVR",NVG_SOURCE_OVER), //destination + source
	new GlobalEffect("DEST-OUT",NVG_DESTINATION_OUT), //destination - source
	new GlobalEffect("LIGHT",NVG_LIGHTER), //destination + source + lighter(source & destination)
	new GlobalEffect("XOR",NVG_XOR), //source ^ destination
	new GlobalEffect("SRC-ATOP",NVG_ATOP), //destination + (source & destination)
	new GlobalEffect("DEST-OVR",NVG_DESTINATION_OVER), //source + destination
	new GlobalEffect("SRC-IN",NVG_SOURCE_IN), //destination & source
	new GlobalEffect("SRC-OUT",NVG_SOURCE_OUT), //source - destination
	new GlobalEffect("DEST-IN",NVG_DESTINATION_IN), //source & destination
	new GlobalEffect("DEST-ATOP",NVG_DESTINATION_ATOP), //source + (destination & source)
	new GlobalEffect("COPY",NVG_COPY) //source
};

// Gets where the point is.
uint8_t GetPointLocationCode(Vec pt, float minX, float maxX, float minY, float maxY)
{
	uint8_t code = POINT_POS_INSIDE;
	if (pt.x < minX)
		code = code | POINT_POS_LEFT;
	else if (pt.x > maxX)
		code = code | POINT_POS_RIGHT;
	if (pt.y < minY)
		code = code | POINT_POS_BOTTOM;
	else if (pt.y > maxY)
		code = code | POINT_POS_TOP;
	return code;
}
// Get where the point is.
uint8_t GetPointLocationCode(Vec pt, Vec minBounds, Vec maxBounds)
{
	uint8_t code = POINT_POS_INSIDE;
	if (pt.x < minBounds.x)
		code = code | POINT_POS_LEFT;
	else if (pt.x > maxBounds.x)
		code = code | POINT_POS_RIGHT;
	if (pt.y < minBounds.y)
		code = code | POINT_POS_BOTTOM;
	else if (pt.y > maxBounds.y)
		code = code | POINT_POS_TOP;
	return code;
}