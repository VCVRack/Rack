#include "mscHack.hpp"

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
RACK_PLUGIN_MODEL_DECLARE(mscHack, Mix_24_4_4);
RACK_PLUGIN_MODEL_DECLARE(mscHack, StepDelay);
RACK_PLUGIN_MODEL_DECLARE(mscHack, PingPong);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Osc_3Ch);
RACK_PLUGIN_MODEL_DECLARE(mscHack, Compressor);

RACK_PLUGIN_INIT(mscHack) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/mschack/VCV-Rack-Plugins");

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
   RACK_PLUGIN_MODEL_ADD(mscHack, Mix_24_4_4);
   RACK_PLUGIN_MODEL_ADD(mscHack, StepDelay);
   RACK_PLUGIN_MODEL_ADD(mscHack, PingPong);
   RACK_PLUGIN_MODEL_ADD(mscHack, Osc_3Ch);
   RACK_PLUGIN_MODEL_ADD(mscHack, Compressor);
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
