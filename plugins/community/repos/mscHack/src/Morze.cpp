#include "mscHack.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mscHack {

#define WAVE_BUFFER_LEN ( 192000 / 20 ) // (9600) based on quality for 20Hz at max sample rate 192000

typedef struct
{
	char code[ 6 ];
}MCODE_STRUCT;

// 1-dot 2-dash
MCODE_STRUCT alphaCode[ 26 ] =
{
	/* A */ { '.', '-', 0, 0, 0, 0 }, /* B */ { '-', '.', '.', '.', 0, 0 }, /* C */ { '-', '.', '-', '.', 0, 0 }, /* D */ { '-', '.', '.', 0, 0, 0 },
	/* E */ { '.', 0, 0, 0, 0, 0 }, /* F */ { '.', '.', '-', '.', 0, 0 }, /* G */ { '-', '-', '.', 0, 0, 0 }, /* H */ { '.', '.', '.', '.', 0, 0 },
	/* I */ { '.', '.', 0, 0, 0, 0 }, /* J */ { '.', '-', '-', '-', 0, 0 }, /* K */ { '-', '.', '-', 0, 0, 0 }, /* L */ { '.', '-', '.', '.', 0, 0 },
	/* M */ { '-', '-', 0, 0, 0, 0 }, /* N */ { '-', '.', 0, 0, 0, 0 }, /* O */ { '-', '-', '-', 0, 0, 0 }, /* P */ { '.', '-', '-', '.', 0, 0 },
	/* Q */ { '-', '-', '.', '-', 0, 0 }, /* R */ { '.', '-', '.', 0, 0, 0 }, /* S */ { '.', '.', '.', 0, 0, 0 }, /* T */ { '-', 0, 0, 0, 0, 0 },
	/* U */ { '.', '.', '-', 0, 0, 0 }, /* V */ { '.', '.', '.', '-', 0, 0 }, /* W */ { '.', '-', '-', 0, 0, 0 }, /* X */ { '-', '.', '.', '-', 0, 0 },
	/* Y */ { '-', '.', '-', '-', 0, 0 }, /* Z */ { '-', '-', '.', '.', 0, 0 }
};

MCODE_STRUCT numCode[ 10 ] =
{
	/* 0 */ { '-', '-', '-', '-', '-', 0 },
	/* '.' */ { '.', '-', '-', '-', '-', 0 },
	/* '-' */ { '.', '.', '-', '-', '-', 0 },
	/* 3 */ { '.', '.', '.', '-', '-', 0 },
	/* 4 */ { '.', '.', '.', '.', '-', 0 },
	/* 5 */ { '.', '.', '.', '.', '.', 0 },
	/* 6 */ { '-', '.', '.', '.', '.', 0 },
	/* 7 */ { '-', '-', '.', '.', '.', 0 },
	/* 8 */ { '-', '-', '-', '.', '.', 0 },
	/* 9 */ { '-', '-', '-', '-', '.', 0 },
};

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Morze : Module
{
	enum ParamIds 
    {
		PARAM_SPEED,
        nPARAMS
    };

	enum InputIds 
    {
		IN_TRIG,
        nINPUTS 
	};

	enum OutputIds 
    {
		OUT_GATE,
        nOUTPUTS
	};

	enum LightIds 
    {
        nLIGHTS
	};

    CLog            lg;
    bool            m_bInitialized = false;

    int             m_Index = 0;
    char            m_Code[ 1024 ] ={};
    int             m_count = 0;
    bool            m_bGate = false;

    std::string     m_stdCurrent;

    SchmittTrigger 	m_SchmitTrig;
    bool            m_bTrigWait = true;

    TextField 		*m_TextField = NULL;

    bool            m_bOneShot = false;

    Label			*m_pTextLabel = NULL;

    // Contructor
    Morze() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

    void    Text2Code( char *strText );
    bool    GetGate( void );

    // Overrides 
	void    step() override;
    void    JsonParams( bool bTo, json_t *root);
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
    void    onCreate() override;
    void    onDelete() override;
};

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Morze_Widget : ModuleWidget {
	Morze_Widget( Morze *module );
};

Morze_Widget::Morze_Widget( Morze *module ) : ModuleWidget(module)
{
	box.size = Vec( 15*5, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Morze.svg")));
		addChild(panel);
	}

    //module->lg.Open("c://users//mark//documents//rack//Morze.txt");

	//mm2px(Vec(3.39962, 14.8373))

	addInput(Port::create<MyPortInSmall>( Vec( 10, 20 ), Port::INPUT, module, Morze::IN_TRIG ) );

	addOutput(Port::create<MyPortOutSmall>( Vec( 48, 20 ), Port::OUTPUT, module, Morze::OUT_GATE ) );

	addParam(ParamWidget::create<Knob_Yellow3_20>( Vec( 10, 280 ), module, Morze::PARAM_SPEED, 0.0f, 1.0f, 0.5f ) );

	module->m_TextField = Widget::create<LedDisplayTextField>( Vec( 4, 100 ) );
	module->m_TextField->box.size = Vec( 67, 150.0 );
	module->m_TextField->multiline = true;
	addChild( module->m_TextField );
	module->m_TextField->text = "mscHack";

	module->m_pTextLabel = new Label();
	module->m_pTextLabel->box.pos = Vec( 30, 250 );
	module->m_pTextLabel->text = "";
	addChild( module->m_pTextLabel );

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	module->Text2Code( (char*)module->m_TextField->text.c_str() );
	//module->BuildWaves();
	module->m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void Morze::JsonParams( bool bTo, json_t *root)
{
    JsonDataString( bTo, "MorseText", root, &m_TextField->text );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Morze::toJson()
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void Morze::fromJson( json_t *root )
{
    JsonParams( FROMJSON, root );

    Text2Code( (char*)m_TextField->text.c_str() );
}

//-----------------------------------------------------
// Procedure:   onCreate
//
//-----------------------------------------------------
void Morze::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void Morze::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void Morze::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void Morze::onRandomize()
{
}

//-----------------------------------------------------
// Procedure:   isalphanum
//
//-----------------------------------------------------
bool isalphanum( char c )
{
	if( c >= 'a' && c <= 'z')
		return true;
	else if( c >= 'A' && c <= 'Z')
		return true;
	else if( c >= '0' && c <= '9')
		return true;

	return false;
}

//-----------------------------------------------------
// Procedure:   Text2Code
//
//-----------------------------------------------------
void Morze::Text2Code( char *strText )
{
	int i = 0;
	char c;
	bool bIgnoreWS =false;
	char strChar[ 2 ] = {0};

	memset( m_Code, 0, sizeof( m_Code ) );

	c = strText[ 0 ];

	while( c )
	{
		strChar[ 0 ] = c;

		if( c >= 'a' && c <= 'z' )
		{
			strcat( m_Code, strChar );
			strcat( m_Code, alphaCode[ c - 'a' ].code );
			strcat( m_Code, "*" );

			bIgnoreWS = false;
		}
		else if( c >= 'A' && c <= 'Z' )
		{
			strcat( m_Code, strChar );
			strcat( m_Code, alphaCode[ c - 'A' ].code );
			strcat( m_Code, "*" );

			bIgnoreWS = false;
		}
		else if( c >= '0' && c <= '9' )
		{
			strcat( m_Code, strChar );
			strcat( m_Code, alphaCode[ c - '0' ].code );
			strcat( m_Code, "*" );

			bIgnoreWS = false;
		}
		else if( c == '.' )
		{
			strcat( m_Code, strChar );
			strcat( m_Code, "." );

			bIgnoreWS = false;
		}
		else if( c == '-' )
		{
			strcat( m_Code, strChar );
			strcat( m_Code, "-" );

			bIgnoreWS = false;
		}
		else
		{
			if( !bIgnoreWS )
			{
				bIgnoreWS = true;
				strcat( m_Code, " " );
			}
		}

		c = strText[ ++i ];
	}

	if( !bIgnoreWS )
	{
		strcat( m_Code, " " );
	}

	strcat( m_Code, "\0" );

	/*i = 0;

	while( m_Code[ i ] )
	{
		lg.f("%c\n", m_Code[ i++ ] );
	};*/

	//lg.f("%s\n", m_Code);

	m_Index = 0;
	m_bGate = false;
	m_count = 0;
	m_bTrigWait = true;
	m_stdCurrent = m_TextField->text;
}

//-----------------------------------------------------
// Procedure:   GetGate
//
//-----------------------------------------------------
bool Morze::GetGate( void )
{
	char strChar[ 2 ] = {0};
	static int spc = 10;
	int ms = (int)( ( engineGetSampleRate() / 1000.0f ) * ( ( 1.0 - params[ PARAM_SPEED ].value ) + 0.5f ) );

	if( --m_count > 0 )
		return m_bGate;

	//break between characters
	if( m_bGate )
	{
		m_count = spc * ms;
		m_bGate = false;
	}
	else
	{
		//lg.f("%d - %c\n", m_Index, m_Code[ m_Index ] );
		//lg.f("%d\n", m_Index );
		switch( m_Code[ m_Index ] )
		{
		case 0:	//end
			m_bGate = false;
			m_count = 0;
			m_Index = 0;
			m_bTrigWait = true;
			return false;

		case '*':	//letter break
			m_bGate = false;
			m_count = 60 * ms;
			break;

		case '.': //dot
			m_count = 80 * ms;
			m_bGate = true;
			spc = 40;
			break;

		case '-': //dash
			m_count = 160 * ms;
			m_bGate = true;
			spc = 80;
			break;

		case ' ':	//word break
			//strChar[ 0 ] = m_Code[ m_Index ];
			//m_pTextLabel->text = strChar;
			m_bGate = false;
			m_count = 400 * ms;
			break;

		default:	// display character
			//lg.f("%d - %c\n", m_Index, m_Code[ m_Index ] );
			strChar[ 0 ] = m_Code[ m_Index ];
			m_pTextLabel->text = strChar;
			m_bGate = false;
			m_count = 0;
			break;
		}

		m_Index ++;
	}

	return m_bGate;
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Morze::step()
{
	static int checkcount = 0;

	if( !m_bInitialized )
		return;

	if( --checkcount <= 0 )
	{
		if( m_stdCurrent != m_TextField->text )
		{
			Text2Code( (char*)m_TextField->text.c_str() );
		}

		checkcount = (int)( engineGetSampleRate() / 10.0f );
	}

	if( m_bTrigWait )
	{
		if( m_SchmitTrig.process( inputs[ IN_TRIG ].normalize( 0.0f ) ) )
		{
			m_bTrigWait = false;
		}
		else
		{
			outputs[ OUT_GATE ].value = 0.0f;
			return;
		}
	}

	if( GetGate() )
		outputs[ OUT_GATE ].value = CV_MAX;
	else
		outputs[ OUT_GATE ].value = 0.0f;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Morze) {
   Model *modelMorze = Model::create<Morze, Morze_Widget>( "mscHack", "Morze", "Morze - Morse Code generator", SEQUENCER_TAG, UTILITY_TAG );
   return modelMorze;
}
