
//
// copied from braids/settings.cc because we can't include that
//

const char* const boolean_values[] = { "OFF ", "ON " };
const char* const intensity_values[] = {
    "OFF ",
    "   1",
    "   2",
    "   3",
    "FULL" };

const char* const zero_to_fifteen_values[] = {
    "   0",
    "   1",
    "   2",
    "   3",
    "   4",
    "   5",
    "   6",
    "   7",
    "   8",
    "   9",
    "  10",
    "  11",
    "  12",
    "  13",
    "  14",
    "  15"};


static const char *algo_values[] = {
	"CSAW",
	"/\\-_",
	"//-_",
	"FOLD",
	"uuuu",
	"SUB-",
	"SUB/",
	"SYN-",
	"SYN/",
	"//x3",
	"-_x3",
	"/\\x3",
	"SIx3",
	"RING",
	"////",
	"//uu",
	"TOY*",
	"ZLPF",
	"ZPKF",
	"ZBPF",
	"ZHPF",
	"VOSM",
	"VOWL",
	"VFOF",
	"HARM",
	"FM  ",
	"FBFM",
	"WTFM",
	"PLUK",
	"BOWD",
	"BLOW",
	"FLUT",
	"BELL",
	"DRUM",
	"KICK",
	"CYMB",
	"SNAR",
	"WTBL",
	"WMAP",
	"WLIN",
	"WTx4",
	"NOIS",
	"TWNQ",
	"CLKN",
	"CLOU",
	"PRTC",
	"QPSK",
	"????", //paques
};

const char* const bits_values[] = {
    "2BIT",
    "3BIT",
    "4BIT",
    "6BIT",
    "8BIT",
    "12B",
    "16B " };
    
const char* const rates_values[] = {
    "4KHZ",
    "8KHZ",
    "16K ",
    "24K ",
    "32K ",
    "48K ",
    "96K " };
    
const char* const quantization_values[] = {
    "OFF ",
    "SEMI",
    "IONI",
    "DORI",
    "PHRY",
    "LYDI",
    "MIXO",
    "AEOL",
    "LOCR",
    "BLU+",
    "BLU-",
    "PEN+",
    "PEN-",
    "FOLK",
    "JAPA",
    "GAME",
    "GYPS",
    "ARAB",
    "FLAM",
    "WHOL",
    "PYTH",
    "EB/4",
    "E /4",
    "EA/4",
    "BHAI",
    "GUNA",
    "MARW",
    "SHRI",
    "PURV",
    "BILA",
    "YAMA",
    "KAFI",
    "BHIM",
    "DARB",
    "RAGE",
    "KHAM",
    "MIMA",
    "PARA",
    "RANG",
    "GANG",
    "KAME",
    "PAKA",
    "NATB",
    "KAUN",
    "BAIR",
    "BTOD",
    "CHAN",
    "KTOD",
    "JOGE" };

const char* const trig_source_values[] = { "EXT.", "AUTO" };

const char* const pitch_range_values[] = {
    "EXT.",
    "FREE",
    "XTND",
    "440 ",
    "LFO "
};

const char* const octave_values[] = { "-2", "-1", "0", "1", "2" };

const char* const trig_delay_values[] = {
    "NONE",
    "125u",
    "250u",
    "500u",
    "1ms ",
    "2ms ",
    "4ms "
};

const char* const brightness_values[] = {
    "\xff   ",
    "\xff\xff  ",
    "\xff\xff\xff\xff",
};

const char* const note_values[] = {
    "C",
    "Db",
    "D",
    "Eb",
    "E",
    "F",
    "Gb",
    "G",
    "Ab",
    "A",
    "Bb",
    "B",
};
