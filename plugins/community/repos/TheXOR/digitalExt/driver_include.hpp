#pragma once
#ifdef LAUNCHPAD
#include "rack.hpp"
#include <map>
#include <list>
//#define DEBUG

/*
 R1C1 is UPPER LEFT
 R8C8 [R16C16] is LOWER RIGHT
 */

#define ALL_LAUNCHPADS     (127)

enum LaunchpadKey
{
    _NOTAKEY = 0,
    RECORD_ARM ,
    TRACK_SELECT,
    MUTE,
    SOLO,
    VOLUME,
    PAN,
    SENDS,
    STOP_CLIP,

    reserved_unused0 = 9,

    RECORD,
    R8C1,R8C2,R8C3,R8C4,R8C5,R8C6,R8C7,R8C8, // 11 ...
    PLAY8,

    kDOUBLE,
    R7C1,R7C2,R7C3,R7C4,R7C5,R7C6,R7C7,R7C8, //21 ...
    PLAY7,
    kDUPLICATE,
    R6C1,R6C2,R6C3,R6C4,R6C5,R6C6,R6C7,R6C8,
    PLAY6,

    QUANTISE,
    R5C1,R5C2,R5C3,R5C4,R5C5,R5C6,R5C7,R5C8,
    PLAY5,

    kDELETE,
    R4C1,R4C2,R4C3,R4C4,R4C5,R4C6,R4C7,R4C8,
    PLAY4,

    UNDO,
    R3C1,R3C2,R3C3,R3C4,R3C5,R3C6,R3C7,R3C8,
    PLAY3,

    CLICK,
    R2C1,R2C2,R2C3,R2C4,R2C5,R2C6,R2C7,R2C8,
    PLAY2,

    SHIFT,
    R1C1,R1C2,R1C3,R1C4,R1C5,R1C6,R1C7,R1C8,
    PLAY1,

    reserved_unused1 = 90,

    UP,
    DOWN,
    LEFT,
    RIGHT,

    SESSION,
    NOTE,
    DEVICE,
    USER
};

enum LaunchpadCommand
{
	_INVALID,
	KEYON,          // key lit, param0 = color (0 = off)
	SET_STANDALONE_MODE,      // set standalone LP mode; param0 = LaunchpadMode
	SET_LIVE_MODE,      // set live LP mode; param0 = LaunchpadLiveMode
	LED_ALL,		// set all keys to one color; param0 = color (0 = off)
	SIDE_LED,       // turn on the side led
	FLASH_KEY,      // led flash
	PULSE_KEY,      // led pulse
	LED_RGB,        //led rgb color
	SETSTATUS,      // param0: ableton or standalone
	RESET,
    SETSCENE,		//currentScene
    REGISTERSCENE,	// currentScene, param0: 1 = register, 0 =unregister
    GETNUMLAUNCHPADS // lpNumber = # di launchpad trovati
};

enum LaunchpadMode {Note = 0, Drum, Fader, Programmer};
enum LaunchpadKeyStatus {keyUp, keyDown, keyPressure, keyChannelPressure, keyNone};
enum LaunchpadScene {SceneAll = 0, Scene1, Scene2, Scene3, Scene4, Scene5, Scene6, Scene7, Scene8};
enum ButtonColorType
{
	Normal = 0,
	RGB,
	Flash,
	Pulse,
};

enum LaunchpadLiveMode 	{Session = 0, DrumRack, ChromaticNote, User, Audio, LiveFader, RecordArm, TrackSelect, Mute, Solo, Volume, Pan, Sends, StopClip};
enum LaunchpadStatus 	{Ableton = 0, Standalone};
struct LaunchpadMessage
{
    LaunchpadKeyStatus status;
    LaunchpadCommand cmd;
    LaunchpadKey key;
    LaunchpadScene currentScene;
    short lpNumber;
    bool shiftDown;
	short param0;
	short param1;
};

struct LaunchpadLed
{
    ButtonColorType status = Normal;
    int r_color = 0;
    int g = 0;
    int b = 0;

    static LaunchpadLed Color(int cr)
    {
        LaunchpadLed rv;
        rv.status = Normal;
        rv.r_color = cr;
        return rv;
    }
    static LaunchpadLed Off() {return Color(0);}

    static LaunchpadLed Rgb(int c_r, int c_g, int c_b)
    {
        LaunchpadLed rv;
        rv.status = RGB;
        rv.r_color = c_r;
        rv.g = c_g;
        rv.b = c_b;
        return rv;
    }
};

#endif
