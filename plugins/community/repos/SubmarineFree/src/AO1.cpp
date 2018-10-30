#include "SubmarineFree.hpp"

namespace rack_plugin_SubmarineFree {

namespace SubmarineAO {

	typedef float (*func_t)(float, float, float);

	struct Functor {
		std::string name;
		func_t func;
	};

#define sMIN(a,b) (((a)>(b))?(b):(a))
#define sMAX(a,b) (((a)>(b))?(a):(b))

#define LAMBDA(e) [](float x, float y, float c)->float { return e ; }
#define X "X"			// X
#define Y "Y"			// Y
#define C "C"			// C
#define A "+"			// Addition symbol
#define S "-"			// Subtraction symbol
#define O "%"			// Modulo symbol
#define OP "("			// Open Parenthesis
#define CP ")"			// Close Parenthesis
#define P "|"			// Pipe symbol
#define M "\xe2\xa8\xaf"	
#define D "\xc3\xb7"		// Division symbol
#define R "\xe2\x88\x9a"	// Root symbol
#define S2 "\xc2\xb2"		// Superscript 2
#define S3 "\xc2\xb3"		// Superscript 3
#define s0 "\xe2\x82\x80"	// Subscript 0
#define s1 "\xe2\x82\x81"	// Subscript 1
#define s2 "\xe2\x82\x82"	// Subscript 2
#define E "\xe2\x84\xaf"	// e
#define SA "\xe2\x81\xba"	// Superscript +
#define SX "\xcb\xa3"		// Superscript x
#define SY "\xca\xb8"		// Superscript y
#define SC "\xe1\xb6\x9c"	// Superscript c
#define MIN "min"		// Minimum function
#define MAX "max"		// Maximum function
#define COMMA ","		// Comma symbol
#define SIN "sin"		// sine function
#define COS "cos"		// cosine function
#define TAN "tan"		// tangent function
#define ASIN "asin"		// arcsine function
#define ACOS "acos"		// arcosine function
#define ATAN "atan"		// arctangent function
#define LOG "log"		// log function
#define LOG2 LOG s2		// base-2 log function
#define LOG10 LOG s1 s0		// base-10 log function
#define IF "if "		// if conditional
#define G ">"			// Greater Than symbol
#define L "<"			// Less Than symbol
#define Q "="			// Equality symbol
#define Z "0"			// Zero
#define W "1"			// One
#define T "\xe2\x86\xa3"	// Right arrow
#define H "/"			// Slash
#define Pi "\xcf\x80"		// PI
#define TAU "\xcf\x84"		// TAU

	std::vector<Functor> functions {
		{ "",                      LAMBDA(  0                      ) }, // Passthrough
		{ C,                       LAMBDA(  c                      ) }, // Addition 
		{ X A C,                   LAMBDA(  x + c                  ) },
		{ Y A C,                   LAMBDA(  y + c                  ) },
		{ X A Y A C,               LAMBDA(  x + y + c              ) },
		{ C S X,                   LAMBDA(  c - x                  ) }, // Subtraction
		{ C S Y,                   LAMBDA(  c - y                  ) },
		{ X S OP Y A C CP,         LAMBDA(  x - ( y + c )          ) },
		{ OP X A C CP S Y,         LAMBDA(  ( x + c ) - y          ) },
		{ Y S OP X A C CP,         LAMBDA(  y - ( x + c )          ) },
		{ OP Y A C CP S X,         LAMBDA(  ( y + c ) - x          ) },
		{ OP X M Y CP A C,         LAMBDA(  ( x * y ) + c          ) }, // Multiplication
		{ OP X A C CP M Y,         LAMBDA(  ( x + c ) * y          ) },
		{ X M OP Y A C CP,         LAMBDA(  x * ( y + c )          ) },
		{ X M C,                   LAMBDA(  x * c                  ) },
		{ Y M C,                   LAMBDA(  y * c                  ) },
		{ X M Y M C,               LAMBDA(  x * y * c              ) },
		{ Pi M OP X A C CP,	   LAMBDA(  M_PI * ( x + c )	   ) },
		{ Pi M OP Y A C CP,	   LAMBDA(  M_PI * ( y + c )	   ) },
		{ TAU M OP X A C CP,	   LAMBDA(  2 * M_PI * ( x + c )   ) },
		{ TAU M OP Y A C CP,	   LAMBDA(  2 * M_PI * ( y + c )   ) },
		{ X D C,                   LAMBDA(  x / c                  ) }, // Division
		{ C D X,                   LAMBDA(  c / x                  ) },
		{ Y D C,                   LAMBDA(  y / c                  ) },
		{ C D Y,                   LAMBDA(  c / y                  ) },
		{ C A OP X D Y CP,         LAMBDA(  c + ( x / y )          ) },
		{ C A OP Y D X CP,         LAMBDA(  c + ( y / x )          ) },
		{ X A OP Y D C CP,	   LAMBDA(  x + ( y / c )	   ) },	
		{ X A OP C D Y CP,	   LAMBDA(  x + ( c / y ) 	   ) },
 		{ Y A OP X D C CP,	   LAMBDA(  y + ( x / c )	   ) },
		{ Y A OP C D X CP,	   LAMBDA(  y + ( c / x ) 	   ) },
		{ OP X A C CP D Y,         LAMBDA(  ( x + c ) / y          ) },
		{ X D OP Y A C CP,         LAMBDA(  x / ( y + c )          ) },
		{ OP Y A C CP D X,         LAMBDA(  ( y + c ) / x          ) },
		{ Y D OP X A C CP,         LAMBDA(  y / ( x + c )          ) },
		{ OP X A C CP O Y,	   LAMBDA(  fmodf( x + c , y )	   ) }, // Modulo
		{ OP Y A C CP O X,	   LAMBDA(  fmodf( y + c , x )	   ) },
		{ X O OP Y A C CP,	   LAMBDA(  fmodf( x , y + c )	   ) },
		{ Y O OP X A C CP,	   LAMBDA(  fmodf( y , x + c)	   ) },
		{ X O C,		   LAMBDA(  fmodf( x , c )	   ) },
		{ Y O C,		   LAMBDA(  fmodf( y , c )  	   ) },
		{ X S2 A C,                LAMBDA(  x * x + c              ) }, // Quadratic
		{ Y S2 A C,                LAMBDA(  y * y + c              ) },
		{ OP X A C CP S2,          LAMBDA(  ( x + c ) * ( x + c )  ) },
		{ OP Y A C CP S2,          LAMBDA(  ( y + c ) * ( y + c )  ) },
		{ X S2 A Y A C,            LAMBDA(  x * x + y + c          ) },
		{ Y S2 A X A C,            LAMBDA(  y * y + x + c          ) },
		{ X S2 A C Y,              LAMBDA(  x * x + c * y          ) },
		{ Y S2 A C X,              LAMBDA(  y * y + c * x          ) },
		{ R OP X A C CP,           LAMBDA(  sqrt( x + c )          ) }, // Square Root
		{ R OP Y A C CP,           LAMBDA(  sqrt( y + c )          ) },
		{ C SX,			   LAMBDA(  powf( c , x )	   ) }, // Powers
		{ C SY,			   LAMBDA(  powf( c , y )	   ) },
		{ C SX SA SY,		   LAMBDA(  powf( c , x + y ) 	   ) },
		{ C SX SY,		   LAMBDA(  powf( c , x * y )	   ) },
		{ X SC,			   LAMBDA(  powf( x , c ) 	   ) },
		{ Y SC,			   LAMBDA(  powf( y , c )	   ) },
		{ X SY SA SC, 		   LAMBDA(  powf( x , y + c )	   ) },
		{ Y SX SA SC,		   LAMBDA(  powf( y , x + c )	   ) },
		{ X SC SY,		   LAMBDA(  powf( x , c * y )	   ) },
		{ Y SC SX,		   LAMBDA(  powf( y , c * x )	   ) },
                { P X A C P,               LAMBDA(  abs( x + c )           ) }, // Modulus
		{ P Y A C P,               LAMBDA(  abs( y + c )           ) },
		{ MIN OP X A C COMMA Y CP, LAMBDA(  sMIN( x + c, y )   ) }, // Minmax
		{ MIN OP X COMMA C CP,     LAMBDA(  sMIN( x, c )       ) },
      		{ MIN OP Y COMMA C CP,     LAMBDA(  sMIN( y, c )       ) },
     		{ MAX OP X A C COMMA Y CP, LAMBDA(  sMAX( x + c, y )   ) },
		{ MAX OP X COMMA C CP,     LAMBDA(  sMAX( x, c )       ) },
		{ MAX OP Y COMMA C CP,     LAMBDA(  sMAX( y, c )       ) },
		{ SIN OP X A C CP,         LAMBDA(  sin( x + c )           ) }, // Trigonometric
		{ SIN OP Y A C CP,         LAMBDA(  sin( y + c )           ) },
		{ SIN OP X A Y CP,         LAMBDA(  sin( x + y )           ) },
 		{ SIN OP C X CP,           LAMBDA(  sin( c * x )           ) },
		{ SIN OP C Y CP,           LAMBDA(  sin( c * y )           ) },
		{ SIN OP X Y CP,           LAMBDA(  sin( x * y )           ) },
		{ COS OP X A C CP,         LAMBDA(  cos( x + c )           ) },
		{ COS OP Y A C CP,         LAMBDA(  cos( y + c )           ) },
		{ COS OP X A Y CP,         LAMBDA(  cos( x + y )           ) },
 		{ COS OP C X CP,           LAMBDA(  cos( c * x )           ) },
		{ COS OP C Y CP,           LAMBDA(  cos( c * y )           ) },
		{ COS OP X Y CP,           LAMBDA(  cos( x * y )           ) },
		{ TAN OP X A C CP,         LAMBDA(  tan( x + c )           ) },
		{ TAN OP Y A C CP,         LAMBDA(  tan( y + c )           ) },
		{ TAN OP X A Y CP,         LAMBDA(  tan( x + y )           ) },
 		{ TAN OP C X CP,           LAMBDA(  tan( c * x )           ) },
		{ TAN OP C Y CP,           LAMBDA(  tan( c * y )           ) },
		{ TAN OP X Y CP,           LAMBDA(  tan( x * y )           ) },
		{ ASIN OP X A C CP,        LAMBDA(  asin( x + c )          ) },
		{ ASIN OP Y A C CP,        LAMBDA(  asin( y + c )          ) },
		{ ASIN OP X A Y CP,        LAMBDA(  asin( x + y )          ) },
 		{ ASIN OP C X CP,          LAMBDA(  asin( c * x )          ) },
		{ ASIN OP C Y CP,          LAMBDA(  asin( c * y )          ) },
		{ ASIN OP X Y CP,          LAMBDA(  asin( x * y )          ) },
		{ ACOS OP X A C CP,        LAMBDA(  acos( x + c )          ) },
		{ ACOS OP Y A C CP,        LAMBDA(  acos( y + c )          ) },
		{ ACOS OP X A Y CP,        LAMBDA(  acos( x + y )          ) },
 		{ ACOS OP C X CP,          LAMBDA(  acos( c * x )          ) },
		{ ACOS OP C Y CP,          LAMBDA(  acos( c * y )          ) },
		{ ACOS OP X Y CP,          LAMBDA(  acos( x * y )          ) },
		{ ATAN OP X A C CP,        LAMBDA(  atan( x + c )          ) },
		{ ATAN OP Y A C CP,        LAMBDA(  atan( y + c )          ) },
		{ ATAN OP X A Y CP,        LAMBDA(  atan( x + y )          ) },
 		{ ATAN OP C X CP,          LAMBDA(  atan( c * x )          ) },
		{ ATAN OP C Y CP,          LAMBDA(  atan( c * y )          ) },
		{ ATAN OP X Y CP,          LAMBDA(  atan( x * y )          ) },
		{ LOG OP X A C CP,         LAMBDA(  log( x + c )           ) }, // Logarithmic
		{ LOG OP Y A C CP,         LAMBDA(  log( y + c )           ) },
		{ LOG2 OP X A C CP,        LAMBDA(  log2( x + c )          ) },
		{ LOG2 OP Y A C CP,        LAMBDA(  log2( y + c )          ) },
		{ LOG10 OP X A C CP,       LAMBDA(  log10( x + c )         ) },
		{ LOG10 OP Y A C CP,       LAMBDA(  log10( y + c )         ) },
		{ E SX SA SC,              LAMBDA(  exp( x + c )           ) }, // Exponential
		{ E SY SA SC,              LAMBDA(  exp( y + c )           ) },
		{ E SC SX,                 LAMBDA(  exp( c * x )           ) },
		{ E SC SY,                 LAMBDA(  exp( c * y )           ) },
		{ "2" SX SA SC,            LAMBDA(  powf( 2, x + c )       ) },
		{ "2" SY SA SC,            LAMBDA(  powf( 2, y + c )       ) },
		{ "2" SC SX,               LAMBDA(  powf( 2, c * x )       ) },
		{ "2" SC SY,               LAMBDA(  powf( 2, c * y )       ) },
		{ "10" SX SA SC,           LAMBDA(  powf( 10, x + c )      ) },
		{ "10" SY SA SC,           LAMBDA(  powf( 10, y + c )      ) },
		{ "10" SC SX,              LAMBDA(  powf( 10, c * x )      ) },
		{ "10" SC SY,              LAMBDA(  powf( 10, c * y )      ) },

		{ IF X G Z T Y H C,	   LAMBDA(  (x > 0) ? y : c	   ) }, // Conditional
		{ IF X L Z T Y H C,	   LAMBDA(  (x < 0) ? y : c	   ) },
		{ IF X Q Z T Y H C,	   LAMBDA(  (x == 0) ? y : c	   ) },
		{ IF X G Z T C H Y,	   LAMBDA(  (x > 0) ? c : y	   ) },
		{ IF X L Z T C H Y,	   LAMBDA(  (x < 0) ? c : y	   ) },
		{ IF X Q Z T C H Y,	   LAMBDA(  (x == 0) ? c : y	   ) },
		{ IF X G Z T W H Z,	   LAMBDA(  (x > 0) ? 1 : 0	   ) },
		{ IF X L Z T W H Z,	   LAMBDA(  (x < 0) ? 1 : 0	   ) },
		{ IF X Q Z T W H Z,	   LAMBDA(  (x == 0) ? 1 : 0	   ) },
		{ IF X G Z T X H C,	   LAMBDA(  (x > 0) ? x : c	   ) },
		{ IF X L Z T X H C,	   LAMBDA(  (x < 0) ? x : c	   ) },
		{ IF X Q Z T X H C,	   LAMBDA(  (x == 0) ? x : c	   ) },
		{ IF X G Z T C H X,	   LAMBDA(  (x > 0) ? c : x	   ) },
		{ IF X L Z T C H X,	   LAMBDA(  (x < 0) ? c : x	   ) },
		{ IF X Q Z T C H X,	   LAMBDA(  (x == 0) ? c : x	   ) },

		{ IF Y G Z T X H C,	   LAMBDA(  (y > 0) ? x : c	   ) },
		{ IF Y L Z T X H C,	   LAMBDA(  (y < 0) ? x : c	   ) },
		{ IF Y Q Z T X H C,	   LAMBDA(  (y == 0) ? x : c	   ) },
		{ IF Y G Z T C H X,	   LAMBDA(  (y > 0) ? c : x	   ) },
		{ IF Y L Z T C H X,	   LAMBDA(  (y < 0) ? c : x	   ) },
		{ IF Y Q Z T C H X,	   LAMBDA(  (y == 0) ? c : x	   ) },
		{ IF Y G Z T W H Z,	   LAMBDA(  (y > 0) ? 1 : 0	   ) },
		{ IF Y L Z T W H Z,	   LAMBDA(  (y < 0) ? 1 : 0	   ) },
		{ IF Y Q Z T W H Z,	   LAMBDA(  (y == 0) ? 1 : 0	   ) },
		{ IF Y G Z T Y H C,	   LAMBDA(  (y > 0) ? y : c	   ) },
		{ IF Y L Z T Y H C,	   LAMBDA(  (y < 0) ? y : c	   ) },
		{ IF Y Q Z T Y H C,	   LAMBDA(  (y == 0) ? y : c	   ) },
		{ IF Y G Z T C H Y,	   LAMBDA(  (y > 0) ? c : y	   ) },
		{ IF Y L Z T C H Y,	   LAMBDA(  (y < 0) ? c : y	   ) },
		{ IF Y Q Z T C H Y,	   LAMBDA(  (y == 0) ? c : y	   ) },

		{ IF X G Y T C H Z,	   LAMBDA(  (x > y) ? c : 0	   ) },
		{ IF X L Y T C H Z,        LAMBDA(  (x < y) ? c : 0	   ) },
		{ IF X Q Y T C H Z,	   LAMBDA(  (x == y) ? c : 0	   ) },
		{ IF Y G X T C H Z,	   LAMBDA(  (y > x) ? c : 0	   ) },
		{ IF Y L X T C H Z,        LAMBDA(  (y < x) ? c : 0	   ) },
		{ IF X G Y T X H Z,	   LAMBDA(  (x > y) ? x : 0	   ) },
		{ IF X L Y T X H Z,        LAMBDA(  (x < y) ? x : 0	   ) },
		{ IF X Q Y T X H Z,	   LAMBDA(  (x == y) ? x : 0	   ) },
		{ IF Y G X T X H Z,	   LAMBDA(  (y > x) ? x : 0	   ) },
		{ IF Y L X T X H Z,        LAMBDA(  (y < x) ? x : 0	   ) },
		{ IF X G Y T Y H Z,	   LAMBDA(  (x > y) ? y : 0	   ) },
		{ IF X L Y T Y H Z,        LAMBDA(  (x < y) ? y : 0	   ) },
		{ IF X Q Y T Y H Z,	   LAMBDA(  (x == y) ? y : 0	   ) },
		{ IF Y G X T Y H Z,	   LAMBDA(  (y > x) ? y : 0	   ) },
		{ IF Y L X T Y H Z,        LAMBDA(  (y < x) ? y : 0	   ) },

		{ IF X G C T Y H Z,	   LAMBDA(  (x > c) ? y : 0	   ) },
		{ IF X L C T Y H Z,        LAMBDA(  (x < c) ? y : 0	   ) },
		{ IF X Q C T Y H Z,	   LAMBDA(  (x == c) ? y : 0	   ) },
		{ IF C G X T Y H Z,	   LAMBDA(  (c > x) ? y : 0	   ) },
		{ IF C L X T Y H Z,        LAMBDA(  (c < x) ? y : 0	   ) },
		{ IF X G C T X H Z,	   LAMBDA(  (x > c) ? x : 0	   ) },
		{ IF X L C T X H Z,        LAMBDA(  (x < c) ? x : 0	   ) },
		{ IF X Q C T X H Z,	   LAMBDA(  (x == c) ? x : 0	   ) },
		{ IF C G X T X H Z,	   LAMBDA(  (c > x) ? x : 0	   ) },
		{ IF C L X T X H Z,        LAMBDA(  (c < x) ? x : 0	   ) },
		{ IF X G C T X H Y,	   LAMBDA(  (x > c) ? x : y	   ) },
		{ IF X L C T X H Y,        LAMBDA(  (x < c) ? x : y	   ) },
		{ IF X Q C T X H Y,	   LAMBDA(  (x == c) ? x : y	   ) },
		{ IF C G X T X H Y,	   LAMBDA(  (c > x) ? x : y	   ) },
		{ IF C L X T X H Y,        LAMBDA(  (c < x) ? x : y	   ) },

		{ IF Y G C T X H Z,	   LAMBDA(  (y > c) ? x : 0	   ) },
		{ IF Y L C T X H Z,        LAMBDA(  (y < c) ? x : 0	   ) },
		{ IF Y Q C T X H Z,	   LAMBDA(  (y == c) ? x : 0	   ) },
		{ IF C G Y T X H Z,	   LAMBDA(  (c > y) ? x : 0	   ) },
		{ IF C L Y T X H Z,        LAMBDA(  (c < y) ? x : 0	   ) },
		{ IF Y G C T Y H Z,	   LAMBDA(  (y > c) ? y : 0	   ) },
		{ IF Y L C T Y H Z,        LAMBDA(  (y < c) ? y : 0	   ) },
		{ IF Y Q C T Y H Z,	   LAMBDA(  (y == c) ? y : 0	   ) },
		{ IF C G Y T Y H Z,	   LAMBDA(  (c > y) ? y : 0	   ) },
		{ IF C L Y T Y H Z,        LAMBDA(  (c < y) ? y : 0	   ) },
		{ IF Y G C T Y H X,	   LAMBDA(  (y > c) ? y : x	   ) },
		{ IF Y L C T Y H X,        LAMBDA(  (y < c) ? y : x	   ) },
		{ IF Y Q C T Y H X,	   LAMBDA(  (y == c) ? y : x	   ) },
		{ IF C G Y T Y H X,	   LAMBDA(  (c > y) ? y : x	   ) },
		{ IF C L Y T Y H X,        LAMBDA(  (c < y) ? y : x	   ) },

	};	

#undef X
#undef Y
#undef C
#undef A
#undef S
#undef O
#undef OP
#undef CP
#undef P
#undef M
#undef D
#undef R
#undef S2
#undef S3
#undef s0
#undef s1
#undef s2
#undef E
#undef SA
#undef SX
#undef SY
#undef SC
#undef COMMA
#undef MIN
#undef MAX
#undef SIN
#undef COS
#undef TAN
#undef ASIN
#undef ACOS
#undef ATAN
#undef LOG
#undef LOG2
#undef LOG10
#undef IF
#undef G
#undef L
#undef Q
#undef Z
#undef W
#undef T
#undef H
#undef Pi
#undef TAU

} // end namespace SubmarineA0

struct AOFuncDisplay : Knob {
	std::shared_ptr<Font> font;
	AOFuncDisplay() {
		box.size.x = 80;
		box.size.y = 15;
		snap = true;
		smooth = false;
		speed = 0.5f;
		font = Font::load(assetGlobal("res/fonts/DejaVuSans.ttf"));
	}
	void draw(NVGcontext *vg) override {
		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgText(vg, 41.5, 13, SubmarineAO::functions[value].name.c_str(), NULL);
	}
};

struct AOConstDisplay : Knob {
	std::shared_ptr<Font> font;
	AOConstDisplay() {
		box.size.x = 80;
		box.size.y = 15;
		snap = true;
		speed = 0.005;
		font = Font::load(assetGlobal("res/fonts/DejaVuSans.ttf"));
	}
	void draw(NVGcontext *vg) override {
		char mtext[41];
		sprintf(mtext, "C=%4.2f", ((int)value)/100.0f);
		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgText(vg, 41.5, 13, mtext, NULL);
	}
};

template <unsigned int x, unsigned int y>
struct AO1 : Module {
	enum ParamIds {
		PARAM_FUNC_1,
		PARAM_CONST_1 = x * y,
		NUM_PARAMS = 2 * x * y
	};
	enum InputIds {
		INPUT_X_1,
		INPUT_Y_1 = x,
		NUM_INPUTS = x + y
	};
	enum OutputIds {
		OUTPUT_X_1,
		OUTPUT_Y_1 = x,
		NUM_OUTPUTS = x + y
	};
	enum LightIds {
		NUM_LIGHTS
	};

	AO1() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override {
		float vx[x];
		for (unsigned int ix = 0; ix < x; ix++) {
			vx[ix] = inputs[INPUT_X_1 + ix].value;
		}
		for (unsigned int iy = 0; iy < y; iy++) {
			float vy = inputs[INPUT_Y_1 + iy].value;
			for (unsigned int ix = 0; ix < x; ix++) {
				unsigned int f = params[PARAM_FUNC_1 + ix + iy * x].value;
				if (f >= SubmarineAO::functions.size())
					f = SubmarineAO::functions.size() - 1;
				if (f > 0)
					vy = vx[ix] = SubmarineAO::functions[f].func(vx[ix], vy, ((int)params[PARAM_CONST_1 + ix + iy * x].value)/100.0f);
				// if f is equal to 0, then both x and y pass (crossing) through the module unchanged.
			}
			outputs[OUTPUT_Y_1 + iy].value = std::isfinite(vy)?vy:0.0f;
		}
		for (unsigned int ix = 0; ix < x; ix++) {
			outputs[OUTPUT_X_1 + ix].value = std::isfinite(vx[ix])?vx[ix]:0.0f;
		}
	}
};

template <unsigned int x, unsigned int y>
struct AOWidget : ModuleWidget {
	AOWidget(AO1<x,y> *module) : ModuleWidget(module) {
		setPanel(SubHelper::LoadPanel(plugin, "AO-1", x*y));
		for (unsigned int ix = 0; ix < x; ix++) {
			addInput(Port::create<SilverPort>(Vec(4, 61 + ix * 46), Port::INPUT, module, AO1<x,y>::INPUT_X_1 + ix));
			addOutput(Port::create<SilverPort>(Vec(46 + y * 90, 61 + ix * 46), Port::OUTPUT, module, AO1<x,y>::OUTPUT_X_1 + ix));
		}
		for (unsigned int iy = 0; iy < y; iy++) {
			addInput(Port::create<SilverPort>(Vec(70 + 90 * iy, 19), Port::INPUT, module, AO1<x,y>::INPUT_Y_1 + iy));
			addOutput(Port::create<SilverPort>(Vec(70 + 90 * iy, 335), Port::OUTPUT, module, AO1<x,y>::OUTPUT_Y_1 + iy));
		}
		for (unsigned int iy = 0; iy < y; iy++) {
			for (unsigned int ix = 0; ix < x; ix++) {
				addParam(ParamWidget::create<AOFuncDisplay>(Vec(42.5 + 90 * iy, 59 + 46 * ix), module, AO1<x,y>::PARAM_FUNC_1 + ix + iy * x, 0.0f, SubmarineAO::functions.size() - 1.0f, 0.0f ));
				addParam(ParamWidget::create<AOConstDisplay>(Vec(42.5 + 90 * iy, 78 + 46 * ix), module, AO1<x,y>::PARAM_CONST_1 + ix + iy * x, -10000.0f, 10000.0f, 0.0f));
			}
		}
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, AO106) {
   Model *modelAO106 = Model::create<AO1<6,1>, AOWidget<6,1>>("Submarine (Free)", "A0-106", "A0-106 Arithmetic Operators", UTILITY_TAG, MULTIPLE_TAG);
   return modelAO106;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, AO112) {
   Model *modelAO112 = Model::create<AO1<6,2>, AOWidget<6,2>>("Submarine (Free)", "A0-112", "A0-112 Arithmetic Operators", UTILITY_TAG, MULTIPLE_TAG);
   return modelAO112;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, AO118) {
   Model *modelAO118 = Model::create<AO1<6,3>, AOWidget<6,3>>("Submarine (Free)", "A0-118", "A0-118 Arithmetic Operators", UTILITY_TAG, MULTIPLE_TAG);
   return modelAO118;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, AO124) {
   Model *modelAO124 = Model::create<AO1<6,4>, AOWidget<6,4>>("Submarine (Free)", "A0-124", "A0-124 Arithmetic Operators", UTILITY_TAG, MULTIPLE_TAG);
   return modelAO124;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, AO136) {
   Model *modelAO136 = Model::create<AO1<6,6>, AOWidget<6,6>>("Submarine (Free)", "A0-136", "A0-136 Arithmetic Operators", UTILITY_TAG, MULTIPLE_TAG);
   return modelAO136;
}
