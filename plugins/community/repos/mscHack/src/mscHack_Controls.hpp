#pragma once

#define PI 3.14159f
#define DEG2RAD( x ) ( ( x ) * ( PI / 180.0f ) )
#define RAD2DEG( x ) ( ( x ) * ( 180.0f / PI ) )

struct CyanValueLight : ModuleLightWidget 
{
	CyanValueLight() 
    {
		addBaseColor( COLOR_CYAN );
    }
};

struct OrangeValueLight : ModuleLightWidget 
{
	OrangeValueLight() 
    {
		addBaseColor( nvgRGB( 242, 79, 0 ) );
	}
};

struct DarkRedValueLight : ModuleLightWidget 
{
	DarkRedValueLight() 
    {
		addBaseColor( nvgRGB(0x70, 0, 0x30) );
	}
};

struct DarkGreenValueLight : ModuleLightWidget 
{
	DarkGreenValueLight() 
    {
		addBaseColor( nvgRGB(0, 0x90, 0x40) );;
	}
};

struct DarkGreen2ValueLight : ModuleLightWidget 
{
	DarkGreen2ValueLight() 
    {
		addBaseColor( nvgRGB(0, 0x40, 0) );;
	}
};

struct DarkYellow2ValueLight : ModuleLightWidget 
{
	DarkYellow2ValueLight() 
    {
		addBaseColor( nvgRGB(0x40, 0x40, 0) );;
	}
};

#define lvl_to_db( x ) ( 20.0 * log10( x ) )
#define db_to_lvl( x ) ( 1.0 / pow( 10, x / 20.0 ) )

typedef struct
{
    union
    {
        unsigned int dwCol;
        unsigned char Col[ 4 ];
    };
}RGB_STRUCT;

#define DWRGB( r, g, b ) (b | g<<8 | r<<16)

typedef struct
{
    int x, y, x2, y2;
}RECT_STRUCT;

typedef struct
{
    float fx, fy, fx2, fy2;
}fRECT_STRUCT;

typedef struct
{
    float fx, fy;
}POINT_STRUCT;

typedef struct
{
    int nUsed;
    POINT_STRUCT p[ 8 ];
}DRAW_VECT_STRUCT;

typedef struct
{
	float fx, fy;
	float fmx; // as in function y = fx + fn
	float fb;
    bool bVert;
    bool bHorz;
    bool bSet;
}fLine;

//-----------------------------------------------------
// Widget_EnvelopeEdit
//-----------------------------------------------------
#define ENVELOPE_VDIV 8
#define ENVELOPE_DIVISIONS 16
#define ENVELOPE_HANDLES ( ENVELOPE_DIVISIONS + 1 )
#define MAX_ENVELOPE_CHANNELS 9

typedef struct
{
    int state, prevstate;
    float fpos;
    float syncInc;
}EE_CTRL;



struct EnvelopeData
{
	enum EnvelopeData_Modes
	{
	    MODE_LOOP,
	    MODE_REVERSE,
	    MODE_PINGPONG,
	    MODE_ONESHOT,
	    MODE_TWOSHOT,
	    nMODES
	};

	enum EnvelopeData_Ranges
	{
	    RANGE_0to5,
	    RANGE_n5to5,
	    RANGE_0to10,
	    RANGE_n10to10,
		RANGE_0to1,
		RANGE_n1to1,
		RANGE_Audio,
	    nRANGES
	};

	enum EnvelopeData_States
	{
	    STATE_RUN,
	    STATE_RUN_REV,
	    STATE_WAIT_TRIG,
	    STATE_WAIT_TRIG_REV,
	    STATE_HOLD,
	    nSTATES
	};

	enum EnvelopeData_Presets
	{
	    PRESET_CLEAR,
		PRESET_HALF,
		PRESET_SIN,
		PRESET_COS,
		PRESET_COS_HALF,
		PRESET_TRI_FULL,
		PRESET_TRI_HALF,
		PRESET_SQR,
		PRESET_SET,
	    nPRESETS
	};

	bool            m_bInitialized=false;
	bool            m_bGateMode = false;
	int             m_Mode = MODE_LOOP;
	int             m_Range= RANGE_0to5;
	float           m_HandleVal[ ENVELOPE_HANDLES ]={};
	fLine           m_Lines[ ENVELOPE_HANDLES ]={};
	float           m_fsegsize = 1.0f;
	float           m_fIndicator = 0.0f;

	EE_CTRL         m_Clock = {};

	EnvelopeData(){};

	void Init( int mode, int range, bool bGate, float fsegsize );
	void Preset( int preset );

    void resetValAll( float val );
    void setVal( int handle, float val );
    void setMode( int Mode );

    void setDataAll( int *pint );
    void getDataAll( int *pint );

    float getActualVal( float val );

    void  line_from_points( float x1, float y1, float x2, float y2, fLine *L );
    float valfromline ( int handle, float x );
    void  recalcLine( int handle );

    bool  process_state( bool bTrig, bool bHold );
    float procStep( bool bTrig, bool bHold );
};


struct Widget_EnvelopeEdit : OpaqueWidget
{
	enum Widget_EnvelopeEdit_TimeDivs
	{
	    TIME_64th,
	    TIME_32nd,
	    TIME_16th,
	    TIME_8th,
	    TIME_4tr,
	    TIME_Bar,
	    nTIMEDIVS
	};

    typedef void EnvelopeEditCALLBACK ( void *pClass, float val );

    EnvelopeData    m_EnvData[ MAX_ENVELOPE_CHANNELS ] = {};

    bool            m_bInitialized=false;

    int             m_TimeDiv[ MAX_ENVELOPE_CHANNELS ]={TIME_64th};

    float           m_divw=0;
    float           m_handleSize=0, m_handleSizeD2;
    int             m_currentChannel = 0;
    int             m_Drag = -1;
    float           m_fband = 0.0f;

    int             m_MaxChannels;

    bool            m_bDrag = false, m_bDraw = false;
    float           m_Drawy = 0.0f;

    bool            m_bClkReset = false;
    int             m_BeatLen = 0;

    RGB_STRUCT      m_HandleCol[ ENVELOPE_HANDLES ];

    EnvelopeEditCALLBACK *m_pCallback = NULL;
    void            *m_pClass = NULL;

    bool            m_bClkd = false;

    Widget_EnvelopeEdit( int x, int y, int w, int h, int handleSize, void *pClass, EnvelopeEditCALLBACK *pCallback, int nchannels );

    void setView( int ch );
    void resetValAll( int ch, float val );
    void setVal( int ch, int handle, float val );
    void setMode( int ch, int Mode );
    void setRange( int ch, int Range );
    void setGateMode( int ch, bool bGate );
    void setTimeDiv( int ch, int timediv );
    void setBeatLen( int len );

    int  getPos( int ch );
    void setPos( int ch, int pos );

    void setDataAll( int *pint );
    void getDataAll( int *pint );

    float getActualVal( int ch, float val );

    bool process_state( int ch, bool bTrig, bool bHold );
    float procStep( int ch, bool bTrig, bool bHold );

    float Val2y( float fval );
    float y2Val( float fy );

    void  recalcLine( int ch, int handle );

    // overrides
    void draw( NVGcontext *vg ) override;
    void onMouseMove( EventMouseMove &e ) override;
    void onMouseDown( EventMouseDown &e ) override;
	void onDragStart(EventDragStart &e) override;
	void onDragEnd(EventDragEnd &e) override;
	void onDragMove(EventDragMove &e) override;
};

//-----------------------------------------------------
// MySimpleKnob
//-----------------------------------------------------

#define MAX_ANGLE 140.0f
#define MIN_ANGLE ( 360.0f - MAX_ANGLE )
#define RANGE_ANGLE (( MAX_ANGLE * 2.0f ) + 1.0f )

struct MySimpleKnob : OpaqueWidget, FramebufferWidget 
{
    bool            m_bInitialized;
    Vec             m_Loc;
    RGB_STRUCT      m_Colour, m_BgColour;
    bool            m_bNegMode;
    float           m_radius;
    int             m_id, m_ch;
    float           m_fVal;
    float           m_fangle;
    Vec             m_Vecpos;
    float           m_max, m_min, m_default;
    int             m_nSteps;
    void            *m_pClass;

    typedef void MySimpleKnobCALLBACK ( void *pClass, int ch, int id, float fval );

    MySimpleKnobCALLBACK *m_pCallback;

    //-----------------------------------------------------
    // Procedure:   constructor
    //-----------------------------------------------------
    MySimpleKnob( void *pClass, float x, float y, float width, unsigned int dwCol, unsigned int dwBgCol, int ch, int id, int nsteps, MySimpleKnobCALLBACK *pCallback, float min, float max, float def )
    {
        m_radius        = width / 2.0;
        m_Loc.x         = x + m_radius;
        m_Loc.y         = y + m_radius;
        m_id            = id;
        m_ch            = ch;
        m_pClass        = pClass;
        m_pCallback     = pCallback;
        m_Colour.dwCol  = dwCol;
        m_BgColour.dwCol= dwBgCol;
        m_nSteps        = nsteps;
        m_max           = max;
        m_min           = min;
        m_default       = def;
        m_fVal          = def;
        m_fangle        = val2a( def );

		box.pos = Vec( x, y );
        box.size = Vec( width, width );

        m_bInitialized  = true;
    }

    //-----------------------------------------------------
    // Procedure:   val2a
    //-----------------------------------------------------
    float val2a( float val )
    {
        float a = 0.0f;

        if( m_nSteps )
        {

        }
        else
        {
            a = RANGE_ANGLE * val;
            a -= MAX_ANGLE;
        }

        return a;
    }

    //-----------------------------------------------------
    // Procedure:   a2val
    //-----------------------------------------------------
    float a2val( float a )
    {
        float val = 0.0f;

        if( m_nSteps )
        {

        }
        else
        {
            a += MAX_ANGLE;
            val = a / RANGE_ANGLE;
        }

        return val;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw( NVGcontext *vg ) override
    {
        float a1, a2;
        float linewidth = m_radius * 0.15f;

        if( !m_bInitialized )
            return;

        a1 = DEG2RAD( 135.0f );
        a2 = DEG2RAD( 45.0f );

        nvgBeginPath( vg );
        nvgCircle( vg, m_radius, m_radius, m_radius );
        nvgFillColor( vg, nvgRGBA( m_Colour.Col[ 2 ], m_Colour.Col[ 1 ], m_Colour.Col[ 0 ], 255 ) );
        nvgFill( vg );

        nvgStrokeWidth( vg, linewidth );

        /*nvgBeginPath( vg );
        nvgStrokeColor( vg, nvgRGBA( m_Colour.Col[ 2 ], m_Colour.Col[ 1 ], m_Colour.Col[ 0 ], 100 ) );
        nvgArc( vg, m_radius, m_radius, m_radius - ( linewidth / 2.0f ), a1, a2, NVG_CW );
        nvgStroke( vg );*/

        a1 = DEG2RAD( 20.0f );
        a2 = DEG2RAD( 20.0f );

        nvgBeginPath( vg );
        nvgStrokeColor( vg, nvgRGBA( m_BgColour.Col[ 2 ], m_BgColour.Col[ 1 ], m_BgColour.Col[ 0 ], 255 ) );
        //nvgStrokeColor( vg, nvgRGBA( 255, 255, 255, 160 ) );
        nvgArc( vg, m_radius, m_radius, m_radius, a1, a2, NVG_CW );
        nvgLineTo( vg, m_radius, m_radius );
        nvgStroke( vg );

        nvgBeginPath( vg );
        nvgCircle( vg, m_radius, m_radius, m_radius * 0.2f );
        nvgFillColor( vg, nvgRGBA( m_Colour.Col[ 2 ], m_Colour.Col[ 1 ], m_Colour.Col[ 0 ], 255 ) );
        nvgFill( vg );

        dirty = false;
    }

    //-----------------------------------------------------
    // Procedure:   setPos
    //-----------------------------------------------------
    void setVal( float val )
    {
        if( !m_bInitialized )
            return;

        if( val < m_min )
            val = m_min;
        else if( val > m_max )
            val = m_max;

        m_fVal = val;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   setCol
    //-----------------------------------------------------
    void setCol( unsigned int dwCol )
    {
        if( !m_bInitialized )
            return;

        m_Colour.dwCol = dwCol;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   setBgCol
    //-----------------------------------------------------
    void setBgCol( unsigned int dwCol )
    {
        if( !m_bInitialized )
            return;

        m_BgColour.dwCol = dwCol;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   onDragStart
    //-----------------------------------------------------
	void onDragStart(EventDragStart &e) override
    {
        e.consumed = true;
    }

    //-----------------------------------------------------
    // Procedure:   onDragEnd
    //-----------------------------------------------------
	void onDragEnd(EventDragEnd &e) override
    {
        e.consumed = true;
    }

    //-----------------------------------------------------
    // Procedure:   onDragMove
    //-----------------------------------------------------
	void onDragMove(EventDragMove &e) override
    {
        m_Vecpos.x = e.mouseRel.x;
        m_Vecpos.y = e.mouseRel.y;

        if( m_pCallback )
            m_pCallback( m_pClass, m_ch, m_id, m_fVal );

        e.consumed = true;
        dirty = true;
    }
};

//-----------------------------------------------------
// MyLED7DigitDisplay
//-----------------------------------------------------
struct MyLED7DigitDisplay : TransparentWidget, FramebufferWidget 
{
    bool            m_bInitialized = false;
    int             m_Type;
    RGB_STRUCT      m_Colour;
    RGB_STRUCT      m_LEDColour;
    int             m_iVal;
    float           m_fVal;
    float           m_fScale;
    float           m_fSpace;
    float           m_MaxDigits;

	enum MyLEDDisplay_Types 
    {
        TYPE_INT,
        TYPE_FLOAT
	};

DRAW_VECT_STRUCT DigitDrawVects[ 8 ] = 
{
    { 8, { {58, 0}, {143, 0}, {148, 5}, {148, 9}, {130, 27}, {68, 27}, {51, 10}, {51, 7} } },                   // top 0 
    { 8, { {39, 17}, {32, 24}, {18, 124}, {24, 130}, {30, 130},  {48, 117},  {59, 33}, {43, 17} } },            // top left 1
    { 8, { {153, 18}, {135, 36}, {124, 118}, {137, 129}, {142, 129}, {151, 122}, {164, 23}, {159, 18} } },      // top right 2
    { 8, { {56, 123}, {35, 137}, {35, 140}, {50, 152}, {111, 152}, {128, 140}, {128, 136}, {114, 123} } },      // middle 3 
    { 8, { {24, 145}, {14, 152}, {1, 251}, {7, 257}, {11, 257}, {31, 239}, {41, 156}, {27, 145} } },            // bottom left 4 
    { 8, { {137, 145}, {117, 158}, {104, 243}, {119, 258}, {122, 258}, {131, 251}, {147, 152}, {140, 145} } },  // bottom right 5
    { 8, { {38, 247}, {16, 265}, {16, 269}, {22, 275}, {104, 275}, {112, 267}, {112, 264}, {98, 247} } },       // bottom 6
    { 4, { {4 + 154, 240}, {0 + 154, 275}, {32 + 154, 275}, {36 + 154, 240}, {0, 0}, {0, 0}, {0, 0}, {0, 0} } },    // dot
};

#define LED_DIGITAL_SCALE_W 160.0f
#define LED_DIGITAL_SCALE_H 275.0f
#define LED_SPACE 210.0f
#define LED_DISPLAY_DIGITS 5

int DigitToDisplay[ 10 ][ 8 ] =
{
    { 6, 0, 1, 2, 4, 5, 6, 0 }, // 0
    { 2, 2, 5, 0, 0, 0, 0, 0 }, // 1
    { 5, 0, 2, 3, 4, 6, 0, 0 }, // 2
    { 6, 0, 2, 3, 5, 6, 0, 0 }, // 3
    { 4, 1, 2, 3, 5, 0, 0, 0 }, // 4
    { 5, 0, 1, 3, 5, 6, 0, 0 }, // 5
    { 6, 0, 1, 3, 4, 5, 6, 0 }, // 6
    { 3, 0, 2, 5, 0, 0, 0, 0 }, // 7
    { 7, 0, 1, 2, 3, 4, 5, 6 }, // 8
    { 6, 0, 1, 2, 3, 5, 6, 0 }, // 9
};

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    MyLED7DigitDisplay( int x, int y, float fscale, int colour, int LEDcolour, int type, int maxdigits )
    {
        int i, j;

        m_Type = type;
        m_Colour.dwCol = colour;
        m_LEDColour.dwCol = LEDcolour;
        m_fScale = fscale;
        m_fSpace = fscale * LED_SPACE;
        m_MaxDigits = maxdigits;

		box.pos = Vec( x, y );

        box.size = Vec( ( ( 5.0f * LED_DIGITAL_SCALE_W ) + ( 4.0f * LED_SPACE ) ) * fscale, LED_DIGITAL_SCALE_H * fscale );

        // rescale the LED vectors
        for( i = 0; i < 8; i++ )
        {
            for( j = 0; j < 8; j++ )
            {
                DigitDrawVects[ i ].p[ j ].fx *= fscale;
                DigitDrawVects[ i ].p[ j ].fy *= fscale;
            }
        }

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetInt
    //-----------------------------------------------------
    void SetInt( int ival )
    {
        if( !m_bInitialized )
            return;

        if( ival == m_iVal )
            return;

        m_iVal = ival;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetFloat
    //-----------------------------------------------------
    void SetFloat( float fval )
    {
        if( !m_bInitialized )
            return;

        if( fval == m_fVal )
            return;

        m_fVal = fval;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   Set
    //-----------------------------------------------------
    void SetLEDCol( int colour )
    {
        m_LEDColour.dwCol = colour;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   Val2Digits
    //-----------------------------------------------------
    void Val2Digits( int *pdigits )
    {
        int temp;

        if( m_Type == TYPE_FLOAT )
            temp = (int)( m_fVal * 100.0 );
        else
            temp = m_iVal;

        pdigits[ 0 ] = temp / 10000;
        temp -= (pdigits[ 0 ] * 10000 );
        pdigits[ 1 ] = temp / 1000;
        temp -= (pdigits[ 1 ] * 1000 );
        pdigits[ 2 ] = temp / 100;
        temp -= (pdigits[ 2 ] * 100 );
        pdigits[ 3 ] = temp / 10;
        temp -= (pdigits[ 3 ] * 10 );
        pdigits[ 4 ] = temp;
    }

    //-----------------------------------------------------
    // Procedure:   drawvect
    //-----------------------------------------------------
    void drawvect( NVGcontext *vg, float fx, float fy, DRAW_VECT_STRUCT *pvect, RGB_STRUCT *pRGB, bool bLeadingZero ) 
    {
        int i;

        if( !m_bInitialized )
            return;

        if( bLeadingZero )
            nvgFillColor(vg, nvgRGBA( pRGB->Col[ 2 ], pRGB->Col[ 1 ], pRGB->Col[ 0 ], 0x40 ) );
        else
            nvgFillColor(vg, nvgRGB( pRGB->Col[ 2 ], pRGB->Col[ 1 ], pRGB->Col[ 0 ] ) );

		nvgBeginPath(vg);

        for( i = 0; i < pvect->nUsed; i++ )
        {
            if( i == 0 )
                nvgMoveTo(vg, pvect->p[ i ].fx + fx, pvect->p[ i ].fy + fy );
            else
		        nvgLineTo(vg, pvect->p[ i ].fx + fx, pvect->p[ i ].fy + fy );
        }
        
        nvgClosePath(vg);
		nvgFill(vg);
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw( NVGcontext *vg ) override
    {
        int digits[ LED_DISPLAY_DIGITS ] = {};
        float xi;
        int i, j;
        bool bLeadingZero = true, bLead;

        if( !m_bInitialized )
            return;

        xi = 0;

        // get digits from value
        Val2Digits( digits );

        // draw digits
        for( i = (LED_DISPLAY_DIGITS - m_MaxDigits); i < LED_DISPLAY_DIGITS; i++ )
        {
            bLead = false;

            if( ( m_Type == TYPE_FLOAT ) && ( i < 2 ) )
                bLead = ( bLeadingZero && digits[ i ] == 0 );
            else if( ( m_Type == TYPE_INT ) && ( i < 4 ) )
                bLead = ( bLeadingZero && digits[ i ] == 0 );

            for( j = 0; j < DigitToDisplay[ digits[ i ] ][ 0 ]; j++ )
                drawvect( vg, xi, 0, &DigitDrawVects[ DigitToDisplay[ digits[ i ] ][ j + 1 ] ], &m_LEDColour, bLead );

            if( digits[ i ] != 0 )
                bLeadingZero = false;

            // draw decimal
            if( i == 2 && m_Type == TYPE_FLOAT )
                drawvect( vg, xi, 0, &DigitDrawVects[ 7 ], &m_LEDColour, false );

            xi += m_fSpace;
        }
	}
};

//-----------------------------------------------------
// MyLEDButtonStrip
//-----------------------------------------------------
#define nMAX_STRIP_BUTTONS 32
struct MyLEDButtonStrip : OpaqueWidget, FramebufferWidget 
{
    typedef void MyLEDButtonStripCALLBACK ( void *pClass, int id, int nbutton, bool bOn );

    bool            m_bInitialized = false;
    int             m_Id;
    int             m_Type;
    int             m_nButtons;
    bool            m_bOn[ nMAX_STRIP_BUTTONS ] = {};
    int             m_ExclusiveOn = 0;
    int             m_HiLightOn = -1;
    RGB_STRUCT      m_Colour;
    RGB_STRUCT      m_LEDColour[ nMAX_STRIP_BUTTONS ] = {};
    float           m_fLEDsize;
    float           m_fLEDsize_d2;

    MyLEDButtonStripCALLBACK *m_pCallback;
    void              *m_pClass;

    RECT_STRUCT     m_Rect[ nMAX_STRIP_BUTTONS ];

	enum MyLEDButton_Types 
    {
        TYPE_EXCLUSIVE,
        TYPE_EXCLUSIVE_WOFF,
        TYPE_INDEPENDANT
	};

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    MyLEDButtonStrip( int x, int y, int w, int h, int space, float LEDsize, int nbuttons, bool bVert, int colour, int LEDcolour, int type, int id, void *pClass, MyLEDButtonStripCALLBACK *pCallback )
    {
        int i;

        if( nbuttons < 0 || nbuttons > nMAX_STRIP_BUTTONS )
            return;

        m_Id = id;
        m_pCallback = pCallback;
        m_pClass = pClass;
        m_Type = type;
        m_Colour.dwCol = colour;
        m_nButtons = nbuttons;
        m_fLEDsize = LEDsize;
        m_fLEDsize_d2 = LEDsize / 2.0f;

		box.pos = Vec( x, y );

        if( bVert )
            box.size = Vec( w, h * ( nbuttons + space ) );
        else
            box.size = Vec( w * ( nbuttons + space ), h );

        x = 0;
        y = 0;

        for( i = 0; i < m_nButtons; i ++)
        {
            m_LEDColour[ i ].dwCol = LEDcolour;

            m_Rect[ i ].x = x;
            m_Rect[ i ].y = y;
            m_Rect[ i ].x2 = x + w - 1;
            m_Rect[ i ].y2 = y + h - 1;

            if( bVert )
                y += space + h;
            else
                x += space + w;
        }

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   Set
    //-----------------------------------------------------
    void SetHiLightOn( int button )
    {
        m_HiLightOn = button;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   Set
    //-----------------------------------------------------
    void Set( int button, bool bOn )
    {
        if( !m_bInitialized || button < 0 )
            return;

        if( m_Type == TYPE_EXCLUSIVE_WOFF )
        {
            if( button > m_nButtons )
                return;

            m_ExclusiveOn = button;
        }
        else
        {
            if( button >= m_nButtons )
                return;

            if( m_Type == TYPE_EXCLUSIVE )
                m_ExclusiveOn = button;

            m_bOn[ button ] = bOn;
        }

        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetLEDCol
    //-----------------------------------------------------
    void SetLEDCol( int button, int colour )
    {
        if( !m_bInitialized || button < 0 || button >= m_nButtons )
            return;

        m_LEDColour[ button ].dwCol = colour;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw(NVGcontext *vg) override
    {
        float xi, yi;
        int i;
        char alpha = 0xFF;

        if( !m_bInitialized )
            return;

        for( i = 0; i < m_nButtons; i ++)
        {
            if( m_HiLightOn == i )
                nvgFillColor( vg, nvgRGB( 255, 255, 255 ) );
            else
                nvgFillColor( vg, nvgRGB( m_Colour.Col[ 2 ], m_Colour.Col[ 1 ], m_Colour.Col[ 0 ] ) );

            // background
		    nvgBeginPath( vg );
            nvgMoveTo(vg, m_Rect[ i ].x, m_Rect[ i ].y );
		    nvgLineTo(vg, m_Rect[ i ].x2, m_Rect[ i ].y );
		    nvgLineTo(vg, m_Rect[ i ].x2, m_Rect[ i ].y2 );
		    nvgLineTo(vg, m_Rect[ i ].x, m_Rect[ i ].y2 );
		    nvgClosePath( vg );
		    nvgFill( vg );

            nvgFillColor( vg, nvgRGB(0x40, 0x40, 0x40) );

            if( m_HiLightOn == i )
                alpha = 0x40;

            if( m_Type == TYPE_EXCLUSIVE_WOFF )
            {
                if( i == ( m_ExclusiveOn - 1 ) )
                    nvgFillColor( vg, nvgRGBA( m_LEDColour[ i ].Col[ 2 ], m_LEDColour[ i ].Col[ 1 ], m_LEDColour[ i ].Col[ 0 ], alpha ) );
            }
            else if( m_Type == TYPE_EXCLUSIVE )
            {
                if( i == m_ExclusiveOn )
                    nvgFillColor( vg, nvgRGBA( m_LEDColour[ i ].Col[ 2 ], m_LEDColour[ i ].Col[ 1 ], m_LEDColour[ i ].Col[ 0 ], alpha ) );
            }
            else
            {
                if( m_bOn[ i ] )
                   nvgFillColor( vg, nvgRGBA( m_LEDColour[ i ].Col[ 2 ], m_LEDColour[ i ].Col[ 1 ], m_LEDColour[ i ].Col[ 0 ], alpha ) );
            }

            xi = ( ( (float)m_Rect[ i ].x2 + (float)m_Rect[ i ].x ) / 2.0f ) - m_fLEDsize_d2;
            yi = ( ( (float)m_Rect[ i ].y2 + (float)m_Rect[ i ].y ) / 2.0f ) - m_fLEDsize_d2;

		    nvgBeginPath( vg );
            nvgMoveTo(vg, xi, yi );
		    nvgLineTo(vg, xi + m_fLEDsize, yi );
		    nvgLineTo(vg, xi + m_fLEDsize, yi + m_fLEDsize );
		    nvgLineTo(vg, xi, yi + m_fLEDsize );
		    nvgClosePath( vg );
		    nvgFill( vg );
        }
	}

    //-----------------------------------------------------
    // Procedure:   isPoint
    //-----------------------------------------------------
    bool isPoint( RECT_STRUCT *prect, int x, int y )
    {
        if( x < prect->x || x > prect->x2 || y < prect->y || y > prect->y2 )
            return false;

        return true;
    }

    //-----------------------------------------------------
    // Procedure:   onMouseDown
    //-----------------------------------------------------
    void onMouseDown( EventMouseDown &e ) override
    {
        int i;
        e.consumed = false;

        if( !m_bInitialized || e.button != 0 )
            return;

        for( i = 0; i < m_nButtons; i++ )
        {
            if( isPoint( &m_Rect[ i ], (int)e.pos.x, (int)e.pos.y ) )
            {
                m_bOn[ i ] = !m_bOn[ i ];

                if( m_Type == TYPE_EXCLUSIVE_WOFF )
                {
                    if( m_ExclusiveOn == ( i + 1 ) )
                        m_ExclusiveOn = 0;
                    else
                        m_ExclusiveOn = i + 1;

                    if( m_pCallback )
                        m_pCallback( m_pClass, m_Id, m_ExclusiveOn, false );
                }
                else
                {
                    Set( i, m_bOn[ i ] );

                    if( m_pCallback )
                        m_pCallback( m_pClass, m_Id, i, m_bOn[ i ] );
                }

                dirty = true;
                e.consumed = true;
                return;
            }
        }

        return;
    }
};

//-----------------------------------------------------
// MyLEDButton
//-----------------------------------------------------
struct MyLEDButton : OpaqueWidget, FramebufferWidget 
{
    typedef void MyLEDButtonCALLBACK ( void *pClass, int id, bool bOn );

    bool            m_bInitialized = false;
    int             m_Id;
    int             m_Type;
    int             m_StepCount = 0;
    bool            m_bOn = false;
    RGB_STRUCT      m_Colour;
    RGB_STRUCT      m_LEDColour;
    float           m_fLEDsize;
    float           m_fLEDsize_d2;

    MyLEDButtonCALLBACK *m_pCallback;
    void              *m_pClass;

    RECT_STRUCT     m_Rect;

	enum MyLEDButton_Types 
    {
        TYPE_SWITCH,
        TYPE_MOMENTARY
	};

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    MyLEDButton( int x, int y, int w, int h, float LEDsize, int colour, int LEDcolour, int type, int id, void *pClass, MyLEDButtonCALLBACK *pCallback )
    {
        m_Id = id;
        m_pCallback = pCallback;
        m_pClass = pClass;
        m_Type = type;
        m_Colour.dwCol = colour;
        m_LEDColour.dwCol = LEDcolour;
        m_fLEDsize = LEDsize;
        m_fLEDsize_d2 = LEDsize / 2.0f;

		box.pos = Vec( x, y );
        box.size = Vec( w, h );

        m_Rect.x = 0;
        m_Rect.y = 0;
        m_Rect.x2 = w - 1;
        m_Rect.y2 = h - 1;

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   Set
    //-----------------------------------------------------
    void Set( bool bOn )
    {
        m_bOn = bOn;
        dirty = true;

        if( m_Type == TYPE_MOMENTARY && bOn )
            m_StepCount = 8;//(int)( engineGetSampleRate() * 0.05 );
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw(NVGcontext *vg) override
    {
        float xi, yi;

        if( !m_bInitialized )
            return;

        nvgFillColor( vg, nvgRGB( m_Colour.Col[ 2 ], m_Colour.Col[ 1 ], m_Colour.Col[ 0 ] ) );

		nvgBeginPath( vg );
        nvgRect( vg, 0, 0, box.size.x - 1, box.size.y - 1 );
		nvgClosePath( vg );
		nvgFill( vg );

        if( !m_bOn )
            nvgFillColor( vg, nvgRGB(0x40, 0x40, 0x40) );
        else
            nvgFillColor( vg, nvgRGB( m_LEDColour.Col[ 2 ], m_LEDColour.Col[ 1 ], m_LEDColour.Col[ 0 ] ) );

        xi = ( ( (float)m_Rect.x2 + (float)m_Rect.x ) / 2.0f ) - m_fLEDsize_d2 ;
        yi = ( ( (float)m_Rect.y2 + (float)m_Rect.y ) / 2.0f ) - m_fLEDsize_d2 ;

		nvgBeginPath( vg );
        nvgRoundedRect( vg, xi, yi, m_fLEDsize, m_fLEDsize, 2.5 );
		nvgClosePath( vg );
		nvgFill( vg );
	}

    //-----------------------------------------------------
    // Procedure:   isPoint
    //-----------------------------------------------------
    bool isPoint( RECT_STRUCT *prect, int x, int y )
    {
        if( x < prect->x || x > prect->x2 || y < prect->y || y > prect->y2 )
            return false;

        return true;
    }

    //-----------------------------------------------------
    // Procedure:   onMouseDown
    //-----------------------------------------------------
    void onMouseDown( EventMouseDown &e ) override
    {
        e.consumed = false;

        if( !m_bInitialized || e.button != 0 )
            return;

        if( isPoint( &m_Rect, (int)e.pos.x, (int)e.pos.y ) )
        {
            m_bOn = !m_bOn;

            if( m_Type == TYPE_MOMENTARY )
            {
                if( m_pCallback )
                {
                    m_bOn = true;
                    m_StepCount = 8;//(int)( engineGetSampleRate() * 0.05 );
                    m_pCallback( m_pClass, m_Id, true );
                }
            }
            else
            {
                if( m_pCallback )
                    m_pCallback( m_pClass, m_Id, m_bOn );
            }

            dirty = true;
            e.consumed = true;
            return;
        }

        return;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void step() override
    {
        if( m_StepCount && ( m_Type == TYPE_MOMENTARY ) )
        {
            if( --m_StepCount <= 0 )
            {
                m_bOn = false;
                m_StepCount = 0;
                dirty = true;
            }
        }

	    FramebufferWidget::step();
    }
};

//-----------------------------------------------------
// SinglePatternClocked32
//-----------------------------------------------------
#define MAX_CLK_PAT 32
#define LEVELS 5
struct SinglePatternClocked32 : OpaqueWidget, FramebufferWidget 
{
    typedef void SINGLEPAT16CALLBACK ( void *pClass, int id, int pat, int level, int maxpat );

    bool            m_bInitialized = false;
    int             m_Id;
    int             m_nLEDs;
    int             m_MaxPat = 0;
    int             m_PatClk = 0;
    int             m_PatSelLevel[ MAX_CLK_PAT ] = {0};
    int             m_StepCount;

    SINGLEPAT16CALLBACK *m_pCallback;
    void              *m_pClass;

    RECT_STRUCT     m_RectsPatSel[ MAX_CLK_PAT ];
    RGB_STRUCT      m_PatCol[ 2 ];
    RECT_STRUCT     m_RectsMaxPat[ MAX_CLK_PAT ];
    RGB_STRUCT      m_MaxCol[ 2 ];

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    SinglePatternClocked32( int x, int y, int w, int h, int mh, int space, int beatspace, int colourPaton, int colourPatoff, int colourMaxon, int colourMaxoff, int nleds, int id, void *pClass, SINGLEPAT16CALLBACK *pCallback )
    {
        int i;

        if ( nleds < 2 || nleds > MAX_CLK_PAT )
            return;

        m_Id = id;
        m_pCallback = pCallback;
        m_pClass = pClass;
        m_nLEDs = nleds;

        m_PatCol[ 0 ].dwCol = colourPatoff;
        m_PatCol[ 1 ].dwCol = colourPaton;

        m_MaxCol[ 0 ].dwCol = colourMaxoff;
        m_MaxCol[ 1 ].dwCol = colourMaxon;

		box.pos = Vec(x, y);

        x = 0;

        for( i = 0; i < m_nLEDs; i++ )
        {
            m_RectsMaxPat[ i ].x = x;
            m_RectsMaxPat[ i ].y = 0;
            m_RectsMaxPat[ i ].x2 = x + w;
            m_RectsMaxPat[ i ].y2 = mh;

            m_RectsPatSel[ i ].x = x;
            m_RectsPatSel[ i ].y = mh + 2;
            m_RectsPatSel[ i ].x2 = x + w;
            m_RectsPatSel[ i ].y2 = ( h + mh ) + 2;

            if( ( i & 0x3 ) == 3 )
                x += ( w + beatspace );
            else
                x += ( w + space );
        }

        box.size = Vec( x, ( h + mh ) + 2 );

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetPatAll
    //-----------------------------------------------------
    void SetPatAll( int *pPat )
    {
        if ( !pPat )
            return;

        for( int i = 0; i < m_nLEDs; i++ )
            m_PatSelLevel[ i ] = pPat[ i ];// & 0x3;

        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   GetPatAll
    //-----------------------------------------------------
    void GetPatAll( int *pPat )
    {
        if ( !pPat )
            return;

        for( int i = 0; i < m_nLEDs; i++ )
            pPat[ i ] = m_PatSelLevel[ i ];

        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetPat
    //-----------------------------------------------------
    void SetPat( int pat )
    {
        if ( pat < 0 || pat >= m_nLEDs )
            return;

        m_PatSelLevel[ pat ] = ( m_PatSelLevel[ pat ] + 1 );

        if( m_PatSelLevel[ pat ] > LEVELS )
            m_PatSelLevel[ pat ] = 0;

        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetPat
    //-----------------------------------------------------
    void ClrPat( int pat )
    {
        if ( pat < 0 || pat >= m_nLEDs )
            return;

        m_PatSelLevel[ pat ] = 0;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   ClockInc
    //-----------------------------------------------------
    bool ClockInc( void )
    {
        m_PatClk ++;

        if ( m_PatClk < 0 || m_PatClk > m_MaxPat || m_PatClk >= m_nLEDs )
            m_PatClk = 0;

        dirty = true;

        if( m_PatClk == 0 )
            return true;

        return false;
    }

    //-----------------------------------------------------
    // Procedure:   ClockReset
    //-----------------------------------------------------
    void ClockReset( void )
    {
        m_PatClk = 0;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetMax
    //-----------------------------------------------------
    void SetMax( int max )
    {
        m_MaxPat = max;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw(NVGcontext *vg) override
    {
        float xi, yi;

        RGB_STRUCT rgb = {0};
        int i;

        if( !m_bInitialized )
            return;

		nvgFillColor(vg, nvgRGBA(0, 0, 0, 0xc0));
		nvgBeginPath(vg);
		nvgMoveTo(vg, -1, -1 );
		nvgLineTo(vg, box.size.x + 1, -1 );
		nvgLineTo(vg, box.size.x + 1, box.size.y + 1 );
		nvgLineTo(vg, -1, box.size.y + 1 );
		nvgClosePath(vg);
		nvgFill(vg);

        nvgStrokeWidth( vg, 0.35 );
        nvgStrokeColor( vg, nvgRGBA( 0xc0, 0xc0, 0xc0, 255 ) );

        for( i = 0; i < m_nLEDs; i++ )
        {
            // max pattern display
            if( i <= m_MaxPat )
                nvgFillColor( vg, nvgRGB( m_MaxCol[ 1 ].Col[ 2 ], m_MaxCol[ 1 ].Col[ 1 ], m_MaxCol[ 1 ].Col[ 0 ] ) );
            else
                nvgFillColor( vg, nvgRGB( m_MaxCol[ 0 ].Col[ 2 ], m_MaxCol[ 0 ].Col[ 1 ], m_MaxCol[ 0 ].Col[ 0 ] ) );

			nvgBeginPath(vg);
            xi = ( ( (float)m_RectsPatSel[ i ].x2 + (float)m_RectsPatSel[ i ].x ) / 2.0f );
			nvgMoveTo(vg, m_RectsMaxPat[ i ].x, m_RectsMaxPat[ i ].y );
			nvgLineTo(vg, m_RectsMaxPat[ i ].x2, m_RectsMaxPat[ i ].y );
			nvgLineTo(vg, xi, m_RectsMaxPat[ i ].y2 );
			//nvgLineTo(vg, m_RectsMaxPat[ i ].x, m_RectsMaxPat[ i ].y2 );
			nvgClosePath(vg);
		    nvgFill(vg);
            //nvgStroke( vg );

            // pattern select
            rgb.Col[ 0 ] = ( ( m_PatCol[ 1 ].Col[ 0 ] * m_PatSelLevel[ i ] ) + ( m_PatCol[ 0 ].Col[ 0 ] * ( LEVELS - m_PatSelLevel[ i ] ) ) ) / LEVELS;
            rgb.Col[ 1 ] = ( ( m_PatCol[ 1 ].Col[ 1 ] * m_PatSelLevel[ i ] ) + ( m_PatCol[ 0 ].Col[ 1 ] * ( LEVELS - m_PatSelLevel[ i ] ) ) ) / LEVELS;
            rgb.Col[ 2 ] = ( ( m_PatCol[ 1 ].Col[ 2 ] * m_PatSelLevel[ i ] ) + ( m_PatCol[ 0 ].Col[ 2 ] * ( LEVELS - m_PatSelLevel[ i ] ) ) ) / LEVELS;

            nvgFillColor( vg, nvgRGB( rgb.Col[ 2 ], rgb.Col[ 1 ], rgb.Col[ 0 ] ) );

			nvgBeginPath(vg);
			nvgMoveTo(vg, m_RectsPatSel[ i ].x, m_RectsPatSel[ i ].y );
			nvgLineTo(vg, m_RectsPatSel[ i ].x2, m_RectsPatSel[ i ].y );
			nvgLineTo(vg, m_RectsPatSel[ i ].x2, m_RectsPatSel[ i ].y2 );
			nvgLineTo(vg, m_RectsPatSel[ i ].x, m_RectsPatSel[ i ].y2 );
			nvgClosePath(vg);
		    nvgFill(vg);
            nvgStroke( vg );

            xi = ( ( (float)m_RectsPatSel[ i ].x2 + (float)m_RectsPatSel[ i ].x ) / 2.0f ) - 2.0f ;
            yi = ( ( (float)m_RectsPatSel[ i ].y2 + (float)m_RectsPatSel[ i ].y ) / 2.0f ) - 2.0f ;

            if( i == m_PatClk )
                nvgFillColor( vg, nvgRGBA( 0, 0xFF, 0, 0xFF ) );
            else
                nvgFillColor( vg, nvgRGBA( 0, 0, 0, 0xFF ) );

			nvgBeginPath(vg);

            nvgMoveTo(vg, xi, yi );
			nvgLineTo(vg, xi + 4.0f, yi );
			nvgLineTo(vg, xi + 4.0f, yi + 4.0f );
			nvgLineTo(vg, xi, yi + 4.0f );

			nvgClosePath(vg);
		    nvgFill(vg);
        }
	}

    //-----------------------------------------------------
    // Procedure:   isPoint
    //-----------------------------------------------------
    bool isPoint( RECT_STRUCT *prect, int x, int y )
    {
        if( x < prect->x || x > prect->x2 || y < prect->y || y > prect->y2 )
            return false;

        return true;
    }

    //-----------------------------------------------------
    // Procedure:   onMouseDown
    //-----------------------------------------------------
    void onMouseDown( EventMouseDown &e ) override
    {
        int i;

        e.consumed = false;

        if( !m_bInitialized )
            return;

        for( i = 0; i < m_nLEDs; i++)
        {
            if( isPoint( &m_RectsPatSel[ i ], (int)e.pos.x, (int)e.pos.y ) )
            {
                if( e.button == 0 )
                    SetPat( i );
                else
                    ClrPat( i );

                if( m_pCallback )
                    m_pCallback( m_pClass, m_Id, i, m_PatSelLevel[ i ], m_MaxPat );

                dirty = true;
                e.consumed = true;
                return;
            }

            else if( isPoint( &m_RectsMaxPat[ i ], (int)e.pos.x, (int)e.pos.y ) )
            {
                m_MaxPat = i;

                if( m_pCallback )
                    m_pCallback( m_pClass, m_Id, i, m_PatSelLevel[ i ], m_MaxPat );

                dirty = true;
                e.consumed = true;
                return;
            }
        }

        return;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void step() override
    {
	    FramebufferWidget::step();
    }
};

//-----------------------------------------------------
// PatternSelectStrip
//-----------------------------------------------------
#define MAX_PAT 32
struct PatternSelectStrip : OpaqueWidget, FramebufferWidget 
{
    typedef void PATCHANGECALLBACK ( void *pClass, int id, int pat, int maxpat );

    bool            m_bInitialized = false;
    int             m_Id;
    int             m_nLEDs;
    int             m_MaxPat = 0;
    int             m_PatSel = 0;
    int             m_PatPending = -1;
    int             m_StepCount;

    PATCHANGECALLBACK *m_pCallback;
    void              *m_pClass;

    RECT_STRUCT     m_RectsMaxPat[ MAX_PAT ];
    RECT_STRUCT     m_RectsPatSel[ MAX_PAT ];
    RGB_STRUCT      m_PatCol[ 2 ];
    RGB_STRUCT      m_MaxCol[ 2 ];

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    PatternSelectStrip( int x, int y, int w, int h, int colourPaton, int colourPatoff, int colourMaxon, int colourMaxoff, int nleds, int id, void *pClass, PATCHANGECALLBACK *pCallback )
    {
        int i;

        if ( nleds < 0 || nleds > MAX_PAT )
            return;

        m_Id = id;
        m_pCallback = pCallback;
        m_pClass = pClass;
        m_nLEDs = nleds;

        m_PatCol[ 0 ].dwCol = colourPatoff;
        m_PatCol[ 1 ].dwCol = colourPaton;

        m_MaxCol[ 0 ].dwCol = colourMaxoff;
        m_MaxCol[ 1 ].dwCol = colourMaxon;

		box.pos = Vec(x, y);
        box.size = Vec( w * m_nLEDs + ( m_nLEDs * 2 ) - 1, ( h * 2 ) + 2 );

        x = 0;

        for( i = 0; i < m_nLEDs; i++ )
        {
            m_RectsMaxPat[ i ].x = x;
            m_RectsMaxPat[ i ].y = 0;
            m_RectsMaxPat[ i ].x2 = x + w;
            m_RectsMaxPat[ i ].y2 = h;

            m_RectsPatSel[ i ].x = x;
            m_RectsPatSel[ i ].y = h + 2;
            m_RectsPatSel[ i ].x2 = x + w;
            m_RectsPatSel[ i ].y2 = ( h * 2 ) + 2;

            x += ( w + 2 );
        }

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetPat
    //-----------------------------------------------------
    void SetPat( int pat, bool bPending )
    {
        if( bPending )
            m_PatPending = pat;
        else
        {
            m_PatPending = -1;
            m_PatSel = pat;
        }

        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   SetMax
    //-----------------------------------------------------
    void SetMax( int max )
    {
        m_MaxPat = max;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw(NVGcontext *vg) override
    {
        int i;
        float xi;

        if( !m_bInitialized )
            return;

		nvgFillColor(vg, nvgRGBA(0, 0, 0, 0xc0));
		nvgBeginPath(vg);
		nvgMoveTo(vg, -1, -1 );
		nvgLineTo(vg, box.size.x + 1, -1 );
		nvgLineTo(vg, box.size.x + 1, box.size.y + 1 );
		nvgLineTo(vg, -1, box.size.y + 1 );
		nvgClosePath(vg);
		nvgFill(vg);

        for( i = 0; i < m_nLEDs; i++ )
        {
            if( i <= m_MaxPat )
                nvgFillColor( vg, nvgRGB( m_MaxCol[ 1 ].Col[ 2 ], m_MaxCol[ 1 ].Col[ 1 ], m_MaxCol[ 1 ].Col[ 0 ] ) );
            else
                nvgFillColor( vg, nvgRGB( m_MaxCol[ 0 ].Col[ 2 ], m_MaxCol[ 0 ].Col[ 1 ], m_MaxCol[ 0 ].Col[ 0 ] ) );



			nvgBeginPath(vg);
            xi = ( ( (float)m_RectsPatSel[ i ].x2 + (float)m_RectsPatSel[ i ].x ) / 2.0f );
			nvgMoveTo(vg, m_RectsMaxPat[ i ].x, m_RectsMaxPat[ i ].y );
			nvgLineTo(vg, m_RectsMaxPat[ i ].x2, m_RectsMaxPat[ i ].y );
			nvgLineTo(vg, xi, m_RectsMaxPat[ i ].y2 );
			nvgClosePath(vg);
		    nvgFill(vg);

            if( m_PatSel == i )
                nvgFillColor( vg, nvgRGB( m_PatCol[ 1 ].Col[ 2 ], m_PatCol[ 1 ].Col[ 1 ], m_PatCol[ 1 ].Col[ 0 ] ) );
            else
                nvgFillColor( vg, nvgRGB( m_PatCol[ 0 ].Col[ 2 ], m_PatCol[ 0 ].Col[ 1 ], m_PatCol[ 0 ].Col[ 0 ] ) );

			nvgBeginPath(vg);
			nvgMoveTo(vg, m_RectsPatSel[ i ].x, m_RectsPatSel[ i ].y );
			nvgLineTo(vg, m_RectsPatSel[ i ].x2, m_RectsPatSel[ i ].y );
			nvgLineTo(vg, m_RectsPatSel[ i ].x2, m_RectsPatSel[ i ].y2 );
			nvgLineTo(vg, m_RectsPatSel[ i ].x, m_RectsPatSel[ i ].y2 );
			nvgClosePath(vg);
		    nvgFill(vg);

            if( m_PatPending == i )
            {
                nvgFillColor( vg, nvgRGBA( m_PatCol[ 1 ].Col[ 2 ], m_PatCol[ 1 ].Col[ 1 ], m_PatCol[ 1 ].Col[ 0 ], 0x50 ) );

			    nvgBeginPath(vg);
			    nvgMoveTo(vg, m_RectsPatSel[ i ].x, m_RectsPatSel[ i ].y );
			    nvgLineTo(vg, m_RectsPatSel[ i ].x2, m_RectsPatSel[ i ].y );
			    nvgLineTo(vg, m_RectsPatSel[ i ].x2, m_RectsPatSel[ i ].y2 );
			    nvgLineTo(vg, m_RectsPatSel[ i ].x, m_RectsPatSel[ i ].y2 );
			    nvgClosePath(vg);
		        nvgFill(vg);
            }
        }
	}

    //-----------------------------------------------------
    // Procedure:   isPoint
    //-----------------------------------------------------
    bool isPoint( RECT_STRUCT *prect, int x, int y )
    {
        if( x < prect->x || x > prect->x2 || y < prect->y || y > prect->y2 )
            return false;

        return true;
    }

    //-----------------------------------------------------
    // Procedure:   onMouseDown
    //-----------------------------------------------------
    void onMouseDown( EventMouseDown &e ) override
    {
        int i;

        e.consumed = false;

        if( !m_bInitialized )
            return;

        if( e.button != 0 )
            return;

        for( i = 0; i < m_nLEDs; i++)
        {
            if( isPoint( &m_RectsMaxPat[ i ], (int)e.pos.x, (int)e.pos.y ) )
            {
                m_MaxPat = i;

                if( m_pCallback )
                    m_pCallback( m_pClass, m_Id, m_PatSel, m_MaxPat );

                dirty = true;
                e.consumed = true;
                return;
            }

            else if( isPoint( &m_RectsPatSel[ i ], (int)e.pos.x, (int)e.pos.y ) )
            {
                m_PatSel = i;

                if( m_pCallback )
                    m_pCallback( m_pClass, m_Id, m_PatSel, m_MaxPat );

                e.consumed = true;
                return;
            }
        }

        return;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void step() override
    {
	    FramebufferWidget::step();
    }
};

//-----------------------------------------------------
// CompressorLEDMeterWidget
//-----------------------------------------------------
#define nDISPLAY_LEDS 10
const float fleveldb[ nDISPLAY_LEDS ] = { 0, 3, 6, 10, 20, 30, 40, 50, 60, 80 };
struct CompressorLEDMeterWidget : TransparentWidget 
{
    bool            m_bInitialized = false;
    bool            m_bOn[ nDISPLAY_LEDS ] = {};
    int             m_StepCount = 0;
    float           m_fLargest = 0;
    RECT_STRUCT     m_Rects[ nDISPLAY_LEDS ];
    RGB_STRUCT      m_ColourOn;
    RGB_STRUCT      m_ColourOff;
    bool            m_bInvert;

    float flevels[ nDISPLAY_LEDS ];

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    CompressorLEDMeterWidget( bool bInvert, int x, int y, int w, int h, int colouron, int colouroff )
    {
        int i;

        m_bInvert = bInvert;
        m_ColourOn.dwCol = colouron;
        m_ColourOff.dwCol = colouroff;

		box.pos = Vec(x, y);
        box.size = Vec( w, h * nDISPLAY_LEDS + ( nDISPLAY_LEDS * 2 ) );

        y = 1;

        for( i = 0; i < nDISPLAY_LEDS; i++ )
        {
            flevels[ i ] = db_to_lvl( fleveldb[ i ] );

            m_Rects[ i ].x = 0;
            m_Rects[ i ].y = y;
            m_Rects[ i ].x2 = w;
            m_Rects[ i ].y2 = y + h;

            y += ( h + 2 );
        }

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw(NVGcontext *vg) override
    {
        int i;

        if( !m_bInitialized )
            return;

		nvgFillColor(vg, nvgRGBA(0, 0, 0, 0xc0));
		nvgBeginPath(vg);
		nvgMoveTo(vg, -1, -1 );
		nvgLineTo(vg, box.size.x + 1, -1 );
		nvgLineTo(vg, box.size.x + 1, box.size.y + 1 );
		nvgLineTo(vg, -1, box.size.y + 1 );
		nvgClosePath(vg);
		nvgFill(vg);

        for( i = 0; i < nDISPLAY_LEDS; i++ )
        {
            if( m_bOn[ i ] )
                nvgFillColor( vg, nvgRGB( m_ColourOn.Col[ 2 ], m_ColourOn.Col[ 1 ], m_ColourOn.Col[ 0 ] ) );
            else
                nvgFillColor( vg, nvgRGB( m_ColourOff.Col[ 2 ], m_ColourOff.Col[ 1 ], m_ColourOff.Col[ 0 ] ) );

			nvgBeginPath(vg);
			nvgMoveTo(vg, m_Rects[ i ].x, m_Rects[ i ].y );
			nvgLineTo(vg, m_Rects[ i ].x2, m_Rects[ i ].y );
			nvgLineTo(vg, m_Rects[ i ].x2, m_Rects[ i ].y2 );
			nvgLineTo(vg, m_Rects[ i ].x, m_Rects[ i ].y2 );
			nvgClosePath(vg);
		    nvgFill(vg);
        }
	}

    //-----------------------------------------------------
    // Procedure:   Process
    //-----------------------------------------------------
    void Process( float level )
    {
        int steptime = (int)( engineGetSampleRate() * 0.05f );
        int i;

        if( !m_bInitialized )
            return;

        if( fabs( level ) > m_fLargest )
            m_fLargest = fabs( level );

        // only process every 1/10th of a second
        if( ++m_StepCount >= steptime )
        {
            m_StepCount = 0;

            if( m_bInvert )
            {
                for( i = 0; i < nDISPLAY_LEDS; i++ )
                {
                    if( m_fLargest >= flevels[ i ] )
                        m_bOn[ ( nDISPLAY_LEDS - 1 ) - i ] = true;
                    else
                        m_bOn[ ( nDISPLAY_LEDS - 1 ) - i ] = false;
                }
            }
            else
            {
                for( i = 0; i < nDISPLAY_LEDS; i++ )
                {
                    if( m_fLargest >= flevels[ i ] )
                        m_bOn[ i ] = true;
                    else
                        m_bOn[ i ] = false;
                }
            }

            m_fLargest = 0.0f;
        }
    }
};

//-----------------------------------------------------
// LEDMeterWidget
//-----------------------------------------------------
struct LEDMeterWidget : TransparentWidget 
{
    bool            m_bInitialized = false;
    bool            m_bOn[ nDISPLAY_LEDS ] = {};
    int             m_space;
    int             m_StepCount = 0;
    bool            m_bVert;
    float           m_fLargest = 0.0;
    RECT_STRUCT     m_Rects[ nDISPLAY_LEDS ];
    RGB_STRUCT      m_ColoursOn[ nDISPLAY_LEDS ];
    RGB_STRUCT      m_ColoursOff[ nDISPLAY_LEDS ];
    float           flevels[ nDISPLAY_LEDS ] = {};

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    LEDMeterWidget( int x, int y, int w, int h, int space, bool bVert )
    {
        int i, xoff = 0, yoff = 0, xpos = 0, ypos = 0;

        m_space = space;
		box.pos = Vec(x, y);

        if( bVert )
        {
            box.size = Vec( w, h * nDISPLAY_LEDS + (m_space * nDISPLAY_LEDS) );
            yoff = h + m_space; 
        }
        else
        {
            box.size = Vec( w * nDISPLAY_LEDS + (m_space * nDISPLAY_LEDS), h );
            xoff = w + m_space;
        }

        for( i = 0; i < nDISPLAY_LEDS; i++ )
        {
            flevels[ i ] = db_to_lvl( fleveldb[ i ] );

            m_Rects[ i ].x = xpos;
            m_Rects[ i ].y = ypos;
            m_Rects[ i ].x2 = xpos + w;
            m_Rects[ i ].y2 = ypos + h;

            // always red
            if( i == 0 )
            {
                m_ColoursOn[ i ].dwCol = DWRGB( 0xFF, 0, 0 );
                m_ColoursOff[ i ].dwCol= DWRGB( 0x80, 0, 0 );
            }
            // yellow
            else if( i < 3 )
            {
                m_ColoursOn[ i ].dwCol = DWRGB( 0xFF, 0xFF, 0 );
                m_ColoursOff[ i ].dwCol= DWRGB( 0x80, 0x80, 0 );
            }
            // green
            else
            {
                m_ColoursOn[ i ].dwCol = DWRGB( 0, 0xFF, 0 );
                m_ColoursOff[ i ].dwCol= DWRGB( 0, 0x80, 0 );
            }

            xpos += xoff;
            ypos += yoff;
        }

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw(NVGcontext *vg) override
    {
        int i;

        if( !m_bInitialized )
            return;

		nvgFillColor(vg, nvgRGBA(0, 0, 0, 0xc0));
		nvgBeginPath(vg);
		nvgMoveTo(vg, -1, -1 );
		nvgLineTo(vg, box.size.x + 1, -1 );
		nvgLineTo(vg, box.size.x + 1, box.size.y + 1 );
		nvgLineTo(vg, -1, box.size.y + 1 );
		nvgClosePath(vg);
		nvgFill(vg);

        for( i = 0; i < nDISPLAY_LEDS; i++ )
        {
            if( m_bOn[ i ] )
                nvgFillColor( vg, nvgRGB( m_ColoursOn[ i ].Col[ 2 ], m_ColoursOn[ i ].Col[ 1 ], m_ColoursOn[ i ].Col[ 0 ] ) );
            else
                nvgFillColor( vg, nvgRGB( m_ColoursOff[ i ].Col[ 2 ], m_ColoursOff[ i ].Col[ 1 ], m_ColoursOff[ i ].Col[ 0 ] ) );

			nvgBeginPath(vg);
			nvgMoveTo(vg, m_Rects[ i ].x, m_Rects[ i ].y );
			nvgLineTo(vg, m_Rects[ i ].x2, m_Rects[ i ].y );
			nvgLineTo(vg, m_Rects[ i ].x2, m_Rects[ i ].y2 );
			nvgLineTo(vg, m_Rects[ i ].x, m_Rects[ i ].y2 );
			nvgClosePath(vg);
		    nvgFill(vg);
        }
	}

    //-----------------------------------------------------
    // Procedure:   Process
    //-----------------------------------------------------
    void Process( float level )
    {
        int steptime = (int)( engineGetSampleRate() * 0.05 );
        int i;

        if( !m_bInitialized )
            return;

        if( fabs( level ) > m_fLargest )
            m_fLargest = fabs( level );

        // only process every 1/10th of a second
        if( ++m_StepCount >= steptime )
        {
            m_StepCount = 0;

            for( i = 0; i < nDISPLAY_LEDS; i++ )
            {
                if( m_fLargest >= flevels[ i ] )
                    m_bOn[ i ] = true;
                else
                    m_bOn[ i ] = false;
            }

            m_fLargest = 0.0;
        }
    }
};

//-----------------------------------------------------
// Keyboard_3Oct_Widget
//-----------------------------------------------------
struct Keyboard_3Oct_Widget : OpaqueWidget, FramebufferWidget
{
    typedef void NOTECHANGECALLBACK ( void *pClass, int kb, int notepressed, int *pnotes, bool bOn, int button );

#define nKEYS 37
#define MAX_MULT_KEYS 16
#define OCT_OFFSET_X 91

    bool    m_bInitialized = false;
    RGB_STRUCT m_rgb_white, m_rgb_black, m_rgb_on;
    CLog *lg;
    int m_MaxMultKeys = 1;
    int m_KeySave[ MAX_MULT_KEYS ] = {0};
    bool m_bKeyOnList[ nKEYS ] = {false};
    int  m_nKeysOn = 0;
    int m_KeyOn = 0;
    RECT_STRUCT keyrects[ nKEYS ] ={};
    NOTECHANGECALLBACK *pNoteChangeCallback = NULL;
    void *m_pClass = NULL;
    int m_nKb;

DRAW_VECT_STRUCT OctaveKeyDrawVects[ nKEYS ] = 
{
    { 6, { {1, 1}, {1, 62}, {12, 62}, {12, 39}, {7, 39}, {7, 1}, {0, 0}, {0, 0} } },
    { 4, { {8, 1}, {8, 38}, {16, 38}, {16, 1},  {0, 0},  {0, 0}, {0, 0}, {0, 0} } },
    { 8, { {17, 1}, {17, 39}, {14, 39}, {14, 62}, {25, 62}, {25, 39}, {22, 39}, {22, 1} } },
    { 4, { {23, 1}, {23, 38}, {31, 38}, {31, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0} } },
    { 6, { {32, 1}, {32, 39}, {27, 39}, {27, 62}, {38, 62}, {38, 1}, {0, 0}, {0, 0} } },
    { 6, { {40, 1}, {40, 62}, {51, 62}, {51, 39}, {46, 39}, {46, 1}, {0, 0}, {0, 0} } },
    { 4, { {47, 1}, {47, 38}, {55, 38}, {55, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0} } },
    { 8, { {56, 1}, {56, 39}, {53, 39}, {53, 62}, {64, 62}, {64, 39}, {60, 39}, {60, 1} } },
    { 4, { {61, 1}, {61, 38}, {69, 38}, {69, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0} } },
    { 8, { {70, 1}, {70, 39}, {66, 39}, {66, 62}, {77, 62}, {77, 39}, {74, 39}, {74, 1} } },
    { 4, { {75, 1}, {75, 38}, {83, 38}, {83, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0} } },
    { 6, { {84, 1}, {84, 39}, {79, 39}, {79, 62}, {90, 62}, {90, 1}, {0, 0}, {0, 0} } },
};

int keysquare_x[ nKEYS ] = 
{
    1, 8, 14, 23, 27, 40, 47, 53, 61, 66, 75, 79
};

DRAW_VECT_STRUCT OctaveKeyHighC [ 1 ] = 
{
    { 5, { {1, 1}, {1, 62}, {12, 62}, {12, 44}, {12, 1}, {0, 0}, {0, 0}, {0, 0} } }
};

    //-----------------------------------------------------
    // Procedure:   Constructor
    //-----------------------------------------------------
    Keyboard_3Oct_Widget( int x, int y, int maxkeysel, int nKb, unsigned int rgbon, void *pClass, NOTECHANGECALLBACK *pcallback, CLog *plog )
    {
        int i, j, oct = 0;

        if( maxkeysel > MAX_MULT_KEYS )
            return;

        m_MaxMultKeys = maxkeysel;
        pNoteChangeCallback = pcallback;
        m_pClass = pClass;
        m_nKb = nKb;
        m_KeyOn = -1;
        m_rgb_on.dwCol = rgbon;

        lg = plog;

		box.pos = Vec(x, y);

        // calc key rects and calculate the remainder of the key vec list
        for( i = 0; i < nKEYS; i++ )
        {
            if( i >= 12 )
            {
                oct = i / 12;

                if( i == (nKEYS-1) )
                {
                    // populate the rest of the key vect table based on the first octave
                    memcpy( &OctaveKeyDrawVects[ i ], &OctaveKeyHighC[ 0 ], sizeof(DRAW_VECT_STRUCT) ); 

                    for( j = 0; j < 8; j++ )
                        OctaveKeyDrawVects[ i ].p[ j ].fx = OctaveKeyHighC[ 0 ].p[ j ].fx + ( OCT_OFFSET_X * oct );
                        
                    keysquare_x[ i ] = keysquare_x[ 0 ] + ( OCT_OFFSET_X * oct );
                }
                else
                {
                    // populate the rest of the key vect table based on the first octave
                    memcpy( &OctaveKeyDrawVects[ i ], &OctaveKeyDrawVects[ i - ( oct * 12 ) ], sizeof(DRAW_VECT_STRUCT) ); 

                    for( j = 0; j < 8; j++ )
                        OctaveKeyDrawVects[ i ].p[ j ].fx = OctaveKeyDrawVects[ i - ( oct * 12 ) ].p[ j ].fx + ( OCT_OFFSET_X * oct );

                    keysquare_x[ i ] = keysquare_x[ i - ( oct * 12 ) ] + ( OCT_OFFSET_X * oct );
                }
            }

            // build the key rects for key press detection
            keyrects[ i ].x = keysquare_x[ i ]- 1;
            keyrects[ i ].y = 1;

            if( OctaveKeyDrawVects[ i ].nUsed == 4 )
            {
                // black key
                keyrects[ i ].x2 = keyrects[ i ].x + 9;
                keyrects[ i ].y2 = keyrects[ i ].y + 43;
            }
            else
            {
                // white key
                keyrects[ i ].x2 = keyrects[ i ].x + 14;
                keyrects[ i ].y2 = keyrects[ i ].y + 67;
            }
        }

        box.size = Vec( keyrects[ (nKEYS-1) ].x2, 62 );

        m_rgb_white.dwCol = DWRGB( 215, 207, 198 );
        m_rgb_black.dwCol = 0;
        memset( m_KeySave, 0xFF, sizeof(m_KeySave) );

        m_bInitialized = true;
    }

    //-----------------------------------------------------
    // Procedure:   addtokeylist
    //-----------------------------------------------------
    void addtokeylist( int key, int button )
    {
        int count = 0;
        bool bOn = false;

        // single key
        if( m_MaxMultKeys == 1 )
        {
            m_KeySave[ 0 ] = key;
            bOn = true;
        }
        else
        {
            // if key is off we are turning it on, check max keys
            if( !m_bKeyOnList[ key ] )
            {
                // ignore if we already have max keys
                if( ( m_nKeysOn + 1 ) > m_MaxMultKeys )
                    return;

                bOn = true;
                m_nKeysOn++;
                m_bKeyOnList[ key ] = true;
            }
            else
            {
                m_nKeysOn--;
                m_bKeyOnList[ key ] = false;
            }

            memset( m_KeySave, 0xFF, sizeof(m_KeySave) );

            // build key list
            for( int i = 0; i < nKEYS; i++ )
            {
                if( m_bKeyOnList[ i ] )
                    m_KeySave[ count++ ] = i;
            }
        }

        if( pNoteChangeCallback )
            pNoteChangeCallback( m_pClass, m_nKb, key, m_KeySave, bOn, button );
    }

    //-----------------------------------------------------
    // Procedure:   isPoint
    //-----------------------------------------------------
    bool isPoint( RECT_STRUCT *prect, int x, int y )
    {
        if( x < prect->x || x > prect->x2 || y < prect->y || y > prect->y2 )
            return false;

        return true;
    }

    //-----------------------------------------------------
    // Procedure:   onMouseDown
    //-----------------------------------------------------
    void onMouseDown( EventMouseDown &e ) override
    {
        int i;

        e.consumed = false;

        if( !m_bInitialized )
            return;

        // check black keys first they are on top
        for( i = 0; i < nKEYS; i++)
        {
            if( OctaveKeyDrawVects[ i ].nUsed == 4 && isPoint( &keyrects[ i ], (int)e.pos.x, (int)e.pos.y ) )
            {
                addtokeylist( i, e.button );
                dirty = true;
                e.consumed = true;
                return;
            }
        }

        // check white keys
        for( i = 0; i < nKEYS; i++)
        {
            if( OctaveKeyDrawVects[ i ].nUsed != 4 && isPoint( &keyrects[ i ], (int)e.pos.x, (int)e.pos.y ) )
            {
                addtokeylist( i, e.button );
                dirty = true;
                e.consumed = true;
                return;
            }
        }

        return;
    }

    //-----------------------------------------------------
    // Procedure:   setkey
    //-----------------------------------------------------
    void setkey( int *pkey ) 
    {
        memset( m_bKeyOnList, 0, sizeof( m_bKeyOnList ) );
        memset( m_KeySave, 0xFF, sizeof( m_KeySave ) );
        m_nKeysOn = 0;

        // build key list
        for( int i = 0; i < m_MaxMultKeys; i++ )
        {
            if( pkey[ i ] != -1 )
            {
                m_nKeysOn++;
                m_bKeyOnList[ pkey[ i ] ] = true;
                m_KeySave[ i ] = pkey[ i ];
            }
        }

        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   setkeyhighlight
    //-----------------------------------------------------
    void setkeyhighlight( int key ) 
    {
        m_KeyOn = key;
        dirty = true;
    }

    //-----------------------------------------------------
    // Procedure:   drawkey
    //-----------------------------------------------------
    void drawkey( NVGcontext *vg, int key, bool bOn )
    {
        int i;

        if( !m_bInitialized )
            return;

        if( key < 0 || key >= nKEYS )
            return;

        if( bOn )
        {
            // hilite on
            if( key == m_KeyOn )
                nvgFillColor( vg, nvgRGBA( m_rgb_on.Col[ 2 ], m_rgb_on.Col[ 1 ], m_rgb_on.Col[ 0 ], 0x80 ) );
            // normal on
            else
                nvgFillColor( vg, nvgRGB( m_rgb_on.Col[ 2 ], m_rgb_on.Col[ 1 ], m_rgb_on.Col[ 0 ] ) );
        }
        else
        {
            if( OctaveKeyDrawVects[ key ].nUsed == 4 )
                nvgFillColor( vg, nvgRGB( m_rgb_black.Col[ 2 ], m_rgb_black.Col[ 1 ], m_rgb_black.Col[ 0 ] ) );
            else
                nvgFillColor( vg, nvgRGB( m_rgb_white.Col[ 2 ], m_rgb_white.Col[ 1 ], m_rgb_white.Col[ 0 ] ) );
        }

        // draw key
		nvgBeginPath(vg);

        for( i = 0; i < OctaveKeyDrawVects[ key ].nUsed; i++ )
        {
            if( i == 0 )
                nvgMoveTo(vg, (float)OctaveKeyDrawVects[ key ].p[ i ].fx, (float)OctaveKeyDrawVects[ key ].p[ i ].fy );
            else
		        nvgLineTo(vg, (float)OctaveKeyDrawVects[ key ].p[ i ].fx, (float)OctaveKeyDrawVects[ key ].p[ i ].fy );
        }
        
        nvgClosePath(vg);
		nvgFill(vg);
    }

    //-----------------------------------------------------
    // Procedure:   draw
    //-----------------------------------------------------
    void draw( NVGcontext *vg ) override
    {
        int key;

        for( key = 0; key < nKEYS; key++ )
            drawkey( vg, key, false );

        for( key = 0; key < m_MaxMultKeys; key++ )
        {
            if( m_KeySave[ key ] != -1 )
                drawkey( vg, m_KeySave[ key ], true );
        }
	}
};

//-----------------------------------------------------
// Procedure:   MySquareButton
//
//-----------------------------------------------------
struct MySquareButton : SVGSwitch, MomentarySwitch 
{
	MySquareButton() 
    {
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_square_button.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

//-----------------------------------------------------
// Procedure:   MySquareButton2
//
//-----------------------------------------------------
struct MySquareButton2 : SVGSwitch, MomentarySwitch 
{
	MySquareButton2() 
    {
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_Square_Button2.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

//-----------------------------------------------------
// Procedure:   PianoWhiteKey
//
//-----------------------------------------------------
struct PianoWhiteKey : SVGSwitch, ToggleSwitch
{
	PianoWhiteKey()
    {
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_WhiteKeyOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_WhiteKeyOn.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

//-----------------------------------------------------
// Procedure:   PianoBlackKey
//
//-----------------------------------------------------
struct PianoBlackKey : SVGSwitch, ToggleSwitch
{
	PianoBlackKey()
    {
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_BlackKeyOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_BlackKeyOn.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

//-----------------------------------------------------
// Procedure:   MyToggle1
//
//-----------------------------------------------------
struct MyToggle1 : SVGSwitch, ToggleSwitch
{
	MyToggle1()
    {
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_3p_vert_simple_01.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_3p_vert_simple_02.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_3p_vert_simple_03.svg")));

		sw->wrap();
		box.size = sw->box.size;
	}
};

//-----------------------------------------------------
// Procedure:   FilterSelectToggle
//
//-----------------------------------------------------
struct FilterSelectToggle : SVGSwitch, ToggleSwitch
{
	FilterSelectToggle()
    {
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_5p_filtersel_01.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_5p_filtersel_02.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_5p_filtersel_03.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_5p_filtersel_04.svg")));
        addFrame(SVG::load(assetPlugin(plugin,"res/mschack_5p_filtersel_05.svg")));

		sw->wrap();
		box.size = sw->box.size;
	}
};

//-----------------------------------------------------
// Sliders
//
//-----------------------------------------------------
struct MySlider_01 : SVGFader 
{
	MySlider_01() 
    {

		Vec margin = Vec(0, 0);
		maxHandlePos = Vec(0, -4).plus(margin);
		minHandlePos = Vec(0, 33).plus(margin);

        background->svg = SVG::load(assetPlugin(plugin,"res/mschack_sliderBG_01.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));

        handle->svg = SVG::load(assetPlugin(plugin,"res/mschack_sliderKNOB_01.svg"));
		handle->wrap();
	}
};

struct Slider02_10x15 : SVGFader 
{
	Slider02_10x15() 
    {

		Vec margin = Vec(4, 0);
		maxHandlePos = Vec(-3, 0).plus(margin);
		minHandlePos = Vec(-3, 60).plus(margin);

        background->svg = SVG::load(assetPlugin(plugin,"res/mschack_sliderBG_02.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));

        handle->svg = SVG::load(assetPlugin(plugin,"res/mschack_Slider02_10x15.svg"));
		handle->wrap();
	}
};

//-----------------------------------------------------
// Procedure:   MyPortInSmall
//
//-----------------------------------------------------
struct MyPortInSmall : SVGPort 
{
	MyPortInSmall() 
    {
        background->svg = SVG::load(assetPlugin(plugin, "res/mschack_PortIn_small.svg" ) );
		background->wrap();
		box.size = background->box.size;
	}
};

//-----------------------------------------------------
// Procedure:   MyPortOutSmall
//
//-----------------------------------------------------
struct MyPortOutSmall : SVGPort 
{
	MyPortOutSmall() 
    {
        background->svg = SVG::load(assetPlugin(plugin, "res/mschack_PortOut_small.svg" ) );
		background->wrap();
		box.size = background->box.size;
	}
};

//-----------------------------------------------------
// Red1
//
//-----------------------------------------------------
struct Knob_Red1_20 : RoundKnob 
{
	Knob_Red1_20() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Red1_20.svg" )));
	}
};

struct Knob_Red1_15 : RoundKnob 
{
	Knob_Red1_15() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Red1_15.svg" )));
	}
};

//-----------------------------------------------------
// Purp1
//
//-----------------------------------------------------
struct Knob_Purp1_20 : RoundKnob 
{
	Knob_Purp1_20() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Purp1_20.svg" )));
	}
};

struct Knob_Purp1_15 : RoundKnob 
{
	Knob_Purp1_15() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Purp1_15.svg" )));
	}
};


//-----------------------------------------------------
// Green1
//
//-----------------------------------------------------
struct Knob_Green1_15 : RoundKnob 
{
	Knob_Green1_15() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Green1_15.svg" )));
	}
};

struct Knob_Green1_40 : RoundKnob 
{
	Knob_Green1_40() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Green1_40.svg" )));
	}
};

//-----------------------------------------------------
// Blue1
//
//-----------------------------------------------------
struct Knob_Blue1_26 : RoundKnob 
{
	Knob_Blue1_26() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue1_26.svg" )));
	}
};

//-----------------------------------------------------
// Blue2
//
//-----------------------------------------------------
struct Knob_Blue2_26 : RoundKnob 
{
	Knob_Blue2_26() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue2_26.svg" )));
	}
};

struct Knob_Blue2_26_Snap : RoundKnob 
{
	Knob_Blue2_26_Snap() 
    {
        snap = true;
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue2_26.svg" )));
	}
};

struct Knob_Blue2_15 : RoundKnob 
{
	Knob_Blue2_15() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue2_15.svg" )));
	}
};

struct Knob_Blue2_40 : RoundKnob 
{
	Knob_Blue2_40() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue2_40.svg" )));
	}
};

struct Knob_Blue2_56 : RoundKnob 
{
	Knob_Blue2_56() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue2_56.svg" )));
		//box.size = Vec(56, 56);
	}
};

//-----------------------------------------------------
// Blue3
//
//-----------------------------------------------------
struct Knob_Blue3_20 : RoundKnob 
{
	Knob_Blue3_20() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue3_20.svg" )));
	}
};

struct Knob_Blue3_15 : RoundKnob 
{
	Knob_Blue3_15() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Blue3_15.svg" )));
	}
};

//-----------------------------------------------------
// Yellow1
//
//-----------------------------------------------------
struct Knob_Yellow1_26 : RoundKnob 
{
	Knob_Yellow1_26() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow1_26.svg" )));
	}
};

struct Knob_Yellow1_15 : RoundKnob 
{
	Knob_Yellow1_15() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow1_15.svg" )));
	}
};

//-----------------------------------------------------
// Yellow2
//
//-----------------------------------------------------
struct Knob_Yellow2_26 : RoundKnob 
{
	Knob_Yellow2_26() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow2_26.svg" )));
	}
};

struct Knob_Yellow2_26_Snap : RoundKnob 
{
	Knob_Yellow2_26_Snap() 
    {
        snap = true;
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow2_26.svg" )));
	}
};

struct Knob_Yellow2_40 : RoundKnob 
{
	Knob_Yellow2_40() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow2_40.svg" )));
	}
};

struct Knob_Yellow2_56 : RoundKnob 
{
	Knob_Yellow2_56() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow2_56.svg" )));
	}
};

struct Knob_Yellow2_56_Snap : RoundKnob 
{
	Knob_Yellow2_56_Snap() 
    {
        snap = true;
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow2_56.svg" )));
	}
};
//-----------------------------------------------------
// Yellow3
//
//-----------------------------------------------------
struct Knob_Yellow3_15 : RoundKnob 
{
	Knob_Yellow3_15() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow3_15.svg" )));
	}
};

struct Knob_Yellow3_20 : RoundKnob 
{
	Knob_Yellow3_20() 
    {
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow3_20.svg" )));
	}
};

struct Knob_Yellow3_20_Snap : RoundKnob 
{
	Knob_Yellow3_20_Snap() 
    {
        snap = true;
        setSVG(SVG::load(assetPlugin(plugin, "res/mschack_Knob_Yellow3_20.svg" )));
	}
};
