#include "mscHack.hpp"

RACK_PLUGIN_MODEL_DECLARE(mscHack, Alienz);
RACK_PLUGIN_MODEL_DECLARE(mscHack, ASAF8);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Dronez);
RACK_PLUGIN_MODEL_DECLARE(mscHack, MasterClockx4);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Seq_3x16x16);
RACK_PLUGIN_MODEL_DECLARE(mscHack, SEQ_6x32x16);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Seq_Triad2);
RACK_PLUGIN_MODEL_DECLARE(mscHack, SEQ_Envelope_8);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Maude_221);
RACK_PLUGIN_MODEL_DECLARE(mscHack, ARP700);
RACK_PLUGIN_MODEL_DECLARE(mscHack, SynthDrums);
RACK_PLUGIN_MODEL_DECLARE(mscHack, XFade);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Mix_1x4_Stereo);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Mix_2x4_Stereo);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Mix_4x4_Stereo);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Mix_9_3_4);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Mix_16_4_4);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Mix_24_4_4);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Morze);
RACK_PLUGIN_MODEL_DECLARE(mscHack, StepDelay);
RACK_PLUGIN_MODEL_DECLARE(mscHack, OSC_WaveMorph_3);
RACK_PLUGIN_MODEL_DECLARE(mscHack, PingPong);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Osc_3Ch);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Compressor);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Windz);

RACK_PLUGIN_INIT(mscHack) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/mschack/VCV-Rack-Plugins");

   RACK_PLUGIN_MODEL_ADD(mscHack, Alienz);
   RACK_PLUGIN_MODEL_ADD(mscHack, ASAF8);
   RACK_PLUGIN_MODEL_ADD(mscHack, Dronez);
   RACK_PLUGIN_MODEL_ADD(mscHack, MasterClockx4);
   RACK_PLUGIN_MODEL_ADD(mscHack, Seq_3x16x16);
   RACK_PLUGIN_MODEL_ADD(mscHack, SEQ_6x32x16);
   RACK_PLUGIN_MODEL_ADD(mscHack, Seq_Triad2);
   RACK_PLUGIN_MODEL_ADD(mscHack, SEQ_Envelope_8);
   RACK_PLUGIN_MODEL_ADD(mscHack, Maude_221);
   RACK_PLUGIN_MODEL_ADD(mscHack, ARP700);
   RACK_PLUGIN_MODEL_ADD(mscHack, SynthDrums);
   RACK_PLUGIN_MODEL_ADD(mscHack, XFade);
   RACK_PLUGIN_MODEL_ADD(mscHack, Mix_1x4_Stereo);
   RACK_PLUGIN_MODEL_ADD(mscHack, Mix_2x4_Stereo);
   RACK_PLUGIN_MODEL_ADD(mscHack, Mix_4x4_Stereo);
   RACK_PLUGIN_MODEL_ADD(mscHack, Mix_9_3_4);
   RACK_PLUGIN_MODEL_ADD(mscHack, Mix_16_4_4);
   RACK_PLUGIN_MODEL_ADD(mscHack, Mix_24_4_4);
   RACK_PLUGIN_MODEL_ADD(mscHack, Morze);
   RACK_PLUGIN_MODEL_ADD(mscHack, StepDelay);
   RACK_PLUGIN_MODEL_ADD(mscHack, OSC_WaveMorph_3);
   RACK_PLUGIN_MODEL_ADD(mscHack, PingPong);
   RACK_PLUGIN_MODEL_ADD(mscHack, Osc_3Ch);
   RACK_PLUGIN_MODEL_ADD(mscHack, Compressor);
   RACK_PLUGIN_MODEL_ADD(mscHack, Windz);
}

namespace rack_plugin_mscHack {

//-----------------------------------------------------
// Procedure: random
//
//-----------------------------------------------------
#define PHI 0x9e3779b9
unsigned int Q[4096], c = 362436;
unsigned int g_myrindex = 4095;

void init_rand( unsigned int seed )
{
    int i;

    Q[0] = seed;
    Q[1] = seed + PHI;
    Q[2] = seed + PHI + PHI;

    for (i = 3; i < 4096; i++)
            Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;

    c = 362436;
    g_myrindex = 4095;
}

unsigned short srand(void)
{
	long long t, a = 18782LL;
    unsigned int x, r = 0xfffffffe;

    g_myrindex = (g_myrindex + 1) & 4095;
    t = a * Q[g_myrindex] + c;
    c = (t >> 32);
    x = t + c;

    if (x < c)
    {
        x++;
        c++;
    }

    return (unsigned short)( ( Q[g_myrindex] = r - x ) & 0xFFFF );
}

float frand(void)
{
	return (float)srand() / (float)0xFFFF;
}

float frand_mm( float fmin, float fmax )
{
	float range = fmax - fmin;

	return fmin + ( frand() * range );
}

bool brand( void )
{
	return ( srand() & 1 );
}

bool frand_perc( float perc )
{
	return ( frand() <= (perc / 100.0f) );
}

//-----------------------------------------------------
// Procedure: JsonDataInt  
//
//-----------------------------------------------------
void JsonDataInt( bool bTo, std::string strName, json_t *root, int *pdata, int len )
{
    int i;
    json_t *jsarray, *js;

    if( !pdata || !root || len <= 0 )
        return;

    if( bTo )
    {
        jsarray = json_array();

        for ( i = 0; i < len; i++ )
        {
	        js = json_integer( pdata[ i ] );
	        json_array_append_new( jsarray, js );
        }

        json_object_set_new( root, strName.c_str(), jsarray );
    }
    else
    {
        jsarray = json_object_get( root, strName.c_str() );

        if( jsarray )
        {
		    for ( i = 0; i < len; i++)
            {
			    js = json_array_get( jsarray, i );

			    if( js )
				    pdata[ i ] = json_integer_value( js );
		    }
        }
    }
}

//-----------------------------------------------------
// Procedure: JsonDataBool  
//
//-----------------------------------------------------
void JsonDataBool( bool bTo, std::string strName, json_t *root, bool *pdata, int len )
{
    int i;
    json_t *jsarray, *js;

    if( !pdata || !root || len <= 0 )
        return;

    if( bTo )
    {
        jsarray = json_array();

        for ( i = 0; i < len; i++ )
        {
	        js = json_boolean( pdata[ i ] );
	        json_array_append_new( jsarray, js );
        }

        json_object_set_new( root, strName.c_str(), jsarray );
    }
    else
    {
        jsarray = json_object_get( root, strName.c_str() );

        if( jsarray )
        {
		    for ( i = 0; i < len; i++)
            {
			    js = json_array_get( jsarray, i );

			    if( js )
				    pdata[ i ] = json_boolean_value( js );
		    }
        }
    }
}

//-----------------------------------------------------
// Procedure: JsonDataString
//
//-----------------------------------------------------
void JsonDataString( bool bTo, std::string strName, json_t *root, std::string *strText )
{
    json_t *textJ;

    if( !root )
        return;

    if( bTo )
    {
    	json_object_set_new( root, strName.c_str(), json_string( strText->c_str() ) );
    }
    else
    {
    	textJ = json_object_get( root, strName.c_str() );

		if( textJ )
			*strText = json_string_value( textJ );
    }
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;
