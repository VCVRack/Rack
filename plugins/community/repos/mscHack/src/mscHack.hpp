#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(mscHack);

#ifdef USE_VST2
#define plugin "mscHack"
#endif // USE_VST2

#include "CLog.h"
#include "mscHack_Controls.hpp"

namespace rack_plugin_mscHack {

#define CV_MAX (10.0f)
#define AUDIO_MAX (6.0f)
#define VOCT_MAX (6.0f)
#define AMP_MAX (2.0f)

#define TOJSON true
#define FROMJSON false

void JsonDataInt( bool bTo, std::string strName, json_t *root, int *pdata, int len );
void JsonDataBool( bool bTo, std::string strName, json_t *root, bool *pdata, int len );
void JsonDataString( bool bTo, std::string strName, json_t *root, std::string *strText );
void init_rand( unsigned int seed );
unsigned short srand(void);
float frand(void);
float frand_mm( float fmin, float max );
bool  brand( void );
bool  frand_perc( float perc );

} // namespace rack_plugin_mscHack
