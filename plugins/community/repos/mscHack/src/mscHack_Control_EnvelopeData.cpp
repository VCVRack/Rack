#include "mscHack.hpp"
#include "window.hpp"

//-----------------------------------------------------
// Procedure:   constructor
//-----------------------------------------------------
void EnvelopeData::Init( int mode, int range, bool bGate, float fsegsize )
{
	m_bInitialized  = false;

    m_Mode = mode;
    m_Range = range;
    m_bGateMode = bGate;
    m_fsegsize = fsegsize;

	for( int hd = 0; hd < ENVELOPE_HANDLES; hd++ )
		m_HandleVal[ hd ] = 0.5f;

    recalcLine( -1 );

    setMode( m_Mode );

    m_bInitialized  = true;
}

//-----------------------------------------------------
// Procedure:   Preset
//-----------------------------------------------------
void EnvelopeData::Preset( int preset )
{
    int i;
    float a, div;

    if( preset < 0 || preset >= EnvelopeData::nPRESETS )
    	return;

    switch( preset )
    {
    case EnvelopeData::PRESET_CLEAR:
        resetValAll( 0.0f );
        break;
    case EnvelopeData::PRESET_SET:
        resetValAll( 1.0f );
        break;
    case EnvelopeData::PRESET_HALF:
        resetValAll( 0.5f );
        break;
    case EnvelopeData::PRESET_SIN:
        div = (float)(ENVELOPE_HANDLES - 1) / (PI * 2.0f);
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            a = ( 1.0f + sinf( (float)i / div ) ) / 2.0f;
            setVal( i, a );
        }
        break;
    case EnvelopeData::PRESET_COS:
        div = (float)(ENVELOPE_HANDLES - 1) / (PI * 2.0f);
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            a = ( 1.0f + cosf( (float)i / div ) ) / 2.0f;
            setVal( i, a );
        }
        break;
    case EnvelopeData::PRESET_COS_HALF:
        div = (float)(ENVELOPE_HANDLES - 1) / (PI * 1.0f);
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            a = ( 1.0f + cosf( (float)i / div ) ) / 2.0f;
            setVal( i, a );
        }
        break;
    case EnvelopeData::PRESET_TRI_FULL:
        div = 1.0f / 16.0f;
        a = 0;
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
        	setVal( i, a );
            a += div;
        }
        break;
    case EnvelopeData::PRESET_TRI_HALF:
        div = 1.0f / 8.0f;
        a = 0;
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
        	setVal( i, a );
            a += div;

            if( i == 8 )
                a = 0.0f;
        }
        break;
    case EnvelopeData::PRESET_SQR:
        a = 0;
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
        	setVal( i, a );

            if( i == 8 )
                a = 1.0f;
        }
        break;
    default:
        break;
    }
}

//-----------------------------------------------------
// Procedure:   getActualVal
//-----------------------------------------------------
float EnvelopeData::getActualVal( float inval )
{
    float val = 0.0f;

    switch( m_Range )
    {
    case EnvelopeData_Ranges::RANGE_0to5:
        val = inval * 5.0f;
        break;
    case EnvelopeData_Ranges::RANGE_n5to5:
        val = ( ( inval * 2 ) - 1.0 ) * 5.0f;
        break;
    case EnvelopeData_Ranges::RANGE_0to10:
        val = inval * 10.0f;
        break;
    case EnvelopeData_Ranges::RANGE_n10to10:
        val = ( ( inval * 2 ) - 1.0 ) * 10.0f;
        break;
    case EnvelopeData_Ranges::RANGE_0to1:
    	val = inval;
    	break;
    case EnvelopeData_Ranges::RANGE_n1to1:
    	val = ( ( inval * 2 ) - 1.0 );
    	break;
    case EnvelopeData_Ranges::RANGE_Audio:
    	val = ( ( inval * 2 ) - 1.0 ) * AUDIO_MAX;
    	break;
    }

    return val;
}

//-----------------------------------------------------
// Function:    line_from_points				
//
//-----------------------------------------------------
void EnvelopeData::line_from_points( float x1, float y1, float x2, float y2, fLine *L )
{
    float m;
	float xdiff, ydiff;

    if( !L )
        return;

    memset( L, 0, sizeof( fLine ) );
    L->bSet  = true;

	xdiff = x2 - x1;
	xdiff = fabs( xdiff );

	ydiff = y2 - y1;
	ydiff = fabs( ydiff );

    // line is vertical
    if( xdiff < 0.000000001 )
    {
        L->fx     = x1;
        L->bVert = true;
        return;
    }
	else if( ydiff < 0.000000001 )
	{
		L->fy     = y1;
        L->bHorz = true;
		return;
	}

	//normal line
    m = (y2 - y1) / (x2 - x1);

    // point slope form
	//y = mx + b
    L->fmx = m;
    L->fb  = y1 - (m * x1);
}

//-----------------------------------------------------
// Procedure:   valfromline
//-----------------------------------------------------
float EnvelopeData::valfromline( int handle, float x )
{
    fLine *L;

    if( m_bGateMode )
        return getActualVal( m_HandleVal[ handle ] );

    L = &m_Lines[ handle ];

    if( L->bHorz )
        return getActualVal( L->fy );

    return getActualVal( (x * L->fmx) + L->fb );
}

//-----------------------------------------------------
// Procedure:   recalcLine
//-----------------------------------------------------
void EnvelopeData::recalcLine( int handle )
{
    float fx1, fx2, fy1, fy2;
    int i;

    // calc all lines
    if( handle == -1 )
    {
		for( int h = 0; h < ENVELOPE_DIVISIONS; h++ )
		{
			for( int delta = -1; delta < 1; delta++ )
			{
				i = ( h + delta ) & 0xF;

				fx1 = (m_fsegsize * i);
				fx2 = fx1 + m_fsegsize;
				fy1 = m_HandleVal[ i ];
				fy2 = m_HandleVal[ i + 1 ];

				line_from_points( fx1, fy1, fx2, fy2, &m_Lines[ i ] );
			}
		}
    }
    // calc line before and line after handle
    else
    {
        for( int delta = -1; delta < 1; delta++ )
        {
            i = ( handle + delta ) & 0xF;

            fx1 = (m_fsegsize * i);
            fx2 = fx1 + m_fsegsize;
            fy1 = m_HandleVal[ i ];
            fy2 = m_HandleVal[ i + 1 ];

            line_from_points( fx1, fy1, fx2, fy2, &m_Lines[ i ] );
        }
    }
}

//-----------------------------------------------------
// Procedure:   resetVal
//-----------------------------------------------------
void EnvelopeData::resetValAll( float val )
{
    if( !m_bInitialized )
        return;

    for( int i = 0; i < ENVELOPE_HANDLES; i++ )
    {
        m_HandleVal[ i ] = val;
    }

    recalcLine( -1 );
}

//-----------------------------------------------------
// Procedure:   setVal
//-----------------------------------------------------
void EnvelopeData::setVal( int handle, float val )
{
    if( !m_bInitialized )
        return;

    m_HandleVal[ handle ] = val;
    recalcLine( handle );
}


//-----------------------------------------------------
// Procedure:   setMode
//-----------------------------------------------------
void EnvelopeData::setMode( int Mode )
{
    if( !m_bInitialized )
        return;

    switch( Mode )
    {
    case MODE_LOOP:
        m_Clock.state = STATE_RUN;
        break;
    case MODE_REVERSE:
        m_Clock.state = STATE_RUN_REV;
        break;
    case MODE_ONESHOT:
        m_Clock.fpos = 0;
        m_Clock.state = STATE_WAIT_TRIG;
        break;
    case MODE_TWOSHOT:
        m_Clock.fpos = 0;
        m_Clock.state = STATE_WAIT_TRIG;
        break;
    case MODE_PINGPONG:
        if( m_Clock.state == STATE_WAIT_TRIG )
            m_Clock.state = STATE_RUN;
        else if( m_Clock.state == STATE_WAIT_TRIG_REV )
            m_Clock.state = STATE_RUN_REV;

        break;

    default:
        return;
    }

    m_Clock.prevstate = m_Clock.state;
    m_Mode = Mode;
}

//-----------------------------------------------------
// Procedure:   setDataAll
//-----------------------------------------------------
void EnvelopeData::setDataAll( int *pint )
{
    int j, count = 0;

    if( !m_bInitialized || !pint )
    {
        return;
    }

	for( j = 0; j < ENVELOPE_HANDLES; j++ )
	{
		m_HandleVal[ j ] = clamp( (float)pint[ count++ ] / 10000.0f, 0.0f, 1.0f );
	}

    // recalc all lines
    recalcLine( -1 );
}

//-----------------------------------------------------
// Procedure:   getDataAll
//-----------------------------------------------------
void EnvelopeData::getDataAll( int *pint )
{
    int j, count = 0;

    if( !m_bInitialized || !pint )
    {
        return;
    }

	for( j = 0; j < ENVELOPE_HANDLES; j++ )
	{
		pint[ count++ ] = (int)( m_HandleVal[ j ] * 10000.0 );
	}
}


//-----------------------------------------------------
// Procedure:   process_state
//-----------------------------------------------------
bool EnvelopeData::process_state( bool bTrig, bool bHold )
{
    switch( m_Clock.state )
    {
    case STATE_RUN:
    case STATE_RUN_REV:

        if( bHold )
        {
            m_Clock.prevstate = m_Clock.state;
            m_Clock.state = STATE_HOLD;
            break;
        }
        
        // run reverse
        if( m_Clock.state == STATE_RUN_REV )
        {
            m_Clock.fpos -= m_Clock.syncInc;

            if( m_Clock.fpos <= 0.0f )
            {
                switch( m_Mode )
                {
                case MODE_TWOSHOT:
                    m_Clock.fpos = 0;
                    m_Clock.state = STATE_WAIT_TRIG;
                    break;

                case MODE_PINGPONG:
                    m_Clock.fpos = -m_Clock.fpos;
                    m_Clock.state = STATE_RUN;
                    break;

                case MODE_REVERSE:
                default:
                    m_Clock.fpos += engineGetSampleRate();
                }
            }
        }
        // run forward
        else
        {
            m_Clock.fpos += m_Clock.syncInc;

            if( m_Clock.fpos >= engineGetSampleRate() )
            {
                switch( m_Mode )
                {
                case MODE_ONESHOT:
                    m_Clock.fpos = engineGetSampleRate() - 1.0f;;
                    m_Clock.state = STATE_WAIT_TRIG;
                    break;
                case MODE_TWOSHOT:
                    m_Clock.fpos = engineGetSampleRate() - 1.0f;
                    m_Clock.state = STATE_WAIT_TRIG_REV;
                    break;
                case MODE_PINGPONG:
                    m_Clock.fpos -= (m_Clock.fpos - engineGetSampleRate()) * 2.0f;
                    m_Clock.state = STATE_RUN_REV;
                    break;
                case MODE_LOOP:
                default:
                    m_Clock.fpos -= engineGetSampleRate();
                }
            }
        }

        break;

    case STATE_WAIT_TRIG:
        if( bTrig )
        {
            m_Clock.fpos = 0;
            m_Clock.state = STATE_RUN;
        }
        break;
    case STATE_WAIT_TRIG_REV:
        if( bTrig )
        {
            m_Clock.fpos = engineGetSampleRate();
            m_Clock.state = STATE_RUN_REV;
        }
        break;

    case STATE_HOLD:
        if( !bHold )
        {
            m_Clock.state = m_Clock.prevstate;
            break;
        }
        break;
    }

    return true;
}

//-----------------------------------------------------
// Procedure:   procStep
//-----------------------------------------------------
float EnvelopeData::procStep( bool bTrig, bool bHold )
{
    int handle;

    if( !m_bInitialized )
    	return 0.0f;

    process_state( bTrig, bHold );

    m_fIndicator = m_Clock.fpos / engineGetSampleRate();

    handle = (int)( m_Clock.fpos / ( engineGetSampleRate() / (float)ENVELOPE_DIVISIONS ) );

    return valfromline( handle, m_fIndicator * m_fsegsize * (float)ENVELOPE_DIVISIONS );
}
