#include "mscHack.hpp"
#include "window.hpp"

//-----------------------------------------------------
// Procedure:   constructor
//-----------------------------------------------------
Widget_EnvelopeEdit::Widget_EnvelopeEdit( int x, int y, int w, int h, int handleSize, void *pClass, EnvelopeEditCALLBACK *pCallback )
{
    int ch, hd;

    if( !pClass || !pCallback )
        return;

    m_pClass = pClass;
    m_pCallback = pCallback;
    m_handleSize = handleSize;

	box.pos = Vec( x, y );
    box.size = Vec( w, h );

    // calc division size (16 divisions, 4 beats x 4 quarter)
    m_divw = ( ( (float)w - (float)ENVELOPE_HANDLES) / (float)ENVELOPE_DIVISIONS ) + 1.0f;

    m_handleSizeD2 = ( (float)handleSize / 2.0f );

    // handle rects
    for( ch = 0; ch < MAX_ENVELOPE_CHANNELS; ch++ )
    {
        for( hd = 0; hd < ENVELOPE_HANDLES; hd++ )
        {
            m_HandleVal[ ch ][ hd ] = 0.5f;
            m_HandleCol[ hd ].dwCol = 0xFFFFFF;
        }
    }

    recalcLine( -1, 0 );

    m_BeatLen = (int)engineGetSampleRate();

    m_bInitialized  = true;
}

//-----------------------------------------------------
// Procedure:   Val2y
//-----------------------------------------------------
float Widget_EnvelopeEdit::Val2y( float fval )
{
    return clamp( box.size.y - ( fval * box.size.y ), 0.0f, box.size.y );
}

//-----------------------------------------------------
// Procedure:   y2Val
//-----------------------------------------------------
float Widget_EnvelopeEdit::y2Val( float fy )
{
    return clamp( 1.0 - (fy / box.size.y ), 0.0f, 1.0f );
}

//-----------------------------------------------------
// Procedure:   y2Val
//-----------------------------------------------------
float Widget_EnvelopeEdit::getActualVal( int ch, float inval )
{
    float val = 0.0f;

    switch( m_Range[ ch ] )
    {
    case Widget_EnvelopeEdit_Ranges::RANGE_0to5:
        val = inval * 5.0f;
        break;
    case Widget_EnvelopeEdit_Ranges::RANGE_n5to5:
        val = ( ( inval * 2 ) - 1.0 ) * 5.0f;
        break;
    case Widget_EnvelopeEdit_Ranges::RANGE_0to10:
        val = inval * 10.0f;
        break;
    case Widget_EnvelopeEdit_Ranges::RANGE_n10to10:
        val = ( ( inval * 2 ) - 1.0 ) * 10.0f;
        break;
    }

    return val;
}

//-----------------------------------------------------
// Function:    line_from_points				
//
//-----------------------------------------------------
void Widget_EnvelopeEdit::line_from_points( float x1, float y1, float x2, float y2, fLine *L )
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
float Widget_EnvelopeEdit::valfromline( int ch, int handle, float x )
{
    fLine *L;

    if( m_bGateMode[ ch ] )
        return getActualVal( ch, m_HandleVal[ ch ][ handle ] );

    L = &m_Lines[ ch ][ handle ];

    if( L->bHorz )
        return getActualVal( ch, L->fy );

    return getActualVal( ch, (x * L->fmx) + L->fb );
}

//-----------------------------------------------------
// Procedure:   recalcLine
//-----------------------------------------------------
void Widget_EnvelopeEdit::recalcLine( int chin, int handle )
{
    float fx1, fx2, fy1, fy2;
    int i;

    // calc all lines
    if( chin == -1 )
    {
        for( int ch = 0; ch < MAX_ENVELOPE_CHANNELS; ch++ )
        {
            for( int h = 0; h < ENVELOPE_DIVISIONS; h++ )
            {
                for( int delta = -1; delta < 1; delta++ )
                {
                    i = ( h + delta ) & 0xF;

                    fx1 = (m_divw * i);
                    fx2 = fx1 + m_divw;
                    fy1 = m_HandleVal[ ch ][ i ];
                    fy2 = m_HandleVal[ ch ][ i + 1 ];

                    line_from_points( fx1, fy1, fx2, fy2, &m_Lines[ ch ][ i ] );
                }
            }
        }
    }
    // calc line before and line after handle
    else
    {
        for( int delta = -1; delta < 1; delta++ )
        {
            i = ( handle + delta ) & 0xF;

            fx1 = (m_divw * i);
            fx2 = fx1 + m_divw;
            fy1 = m_HandleVal[ chin ][ i ];
            fy2 = m_HandleVal[ chin ][ i + 1 ];

            line_from_points( fx1, fy1, fx2, fy2, &m_Lines[ chin ][ i ] );
        }
    }
}

//-----------------------------------------------------
// Procedure:   setPos
//-----------------------------------------------------
void Widget_EnvelopeEdit::setView( int ch )
{
    if( !m_bInitialized && ch < MAX_ENVELOPE_CHANNELS && ch >= 0 )
        return;

    m_currentChannel = ch;
}

//-----------------------------------------------------
// Procedure:   resetVal
//-----------------------------------------------------
void Widget_EnvelopeEdit::resetValAll( int ch, float val )
{
    if( !m_bInitialized && ch < MAX_ENVELOPE_CHANNELS && ch >= 0 )
        return;

    for( int i = 0; i < ENVELOPE_HANDLES; i++ )
    {
        m_HandleVal[ ch ][ i ] = val;
    }

    recalcLine( -1, 0 );
}

//-----------------------------------------------------
// Procedure:   setVal
//-----------------------------------------------------
void Widget_EnvelopeEdit::setVal( int ch, int handle, float val )
{
    if( !m_bInitialized && ch < MAX_ENVELOPE_CHANNELS && ch >= 0 )
        return;

    m_HandleVal[ ch ][ handle ] = val;
    recalcLine( ch, handle );
}

//-----------------------------------------------------
// Procedure:   setRange
//-----------------------------------------------------
void Widget_EnvelopeEdit::setRange( int ch, int Range )
{
    if( !m_bInitialized && ch < MAX_ENVELOPE_CHANNELS && ch >= 0 )
        return;

    m_Range[ ch ] = Range;
}

//-----------------------------------------------------
// Procedure:   setMode
//-----------------------------------------------------
void Widget_EnvelopeEdit::setMode( int ch, int Mode )
{
    if( !m_bInitialized && ch < MAX_ENVELOPE_CHANNELS && ch >= 0 )
        return;

    switch( Mode )
    {
    case MODE_LOOP:
        m_Clock[ ch ].state = STATE_RUN;
        break;
    case MODE_REVERSE:
        m_Clock[ ch ].state = STATE_RUN_REV;
        break;
    case MODE_ONESHOT:
        m_Clock[ ch ].fpos = 0;
        m_Clock[ ch ].state = STATE_WAIT_TRIG;
        break;
    case MODE_TWOSHOT:
        m_Clock[ ch ].fpos = 0;
        m_Clock[ ch ].state = STATE_WAIT_TRIG;
        break;
    case MODE_PINGPONG:
        if( m_Clock[ ch ].state == STATE_WAIT_TRIG )
            m_Clock[ ch ].state = STATE_RUN;
        else if( m_Clock[ ch ].state == STATE_WAIT_TRIG_REV )
            m_Clock[ ch ].state = STATE_RUN_REV;

        break;

    default:
        return;
    }

    m_Clock[ ch ].prevstate = m_Clock[ ch ].state;
    m_Mode[ ch ] = Mode;
}

//-----------------------------------------------------
// Procedure:   setGateMode
//-----------------------------------------------------
void Widget_EnvelopeEdit::setGateMode( int ch, bool bGate )
{
    if( !m_bInitialized && ch < MAX_ENVELOPE_CHANNELS && ch >= 0 )
        return;

    m_bGateMode[ ch ] = bGate;
}

//-----------------------------------------------------
// Procedure:   setTimeDiv
//-----------------------------------------------------
void Widget_EnvelopeEdit::setTimeDiv( int ch, int timediv )
{
    if( !m_bInitialized && ch < MAX_ENVELOPE_CHANNELS && ch >= 0 )
        return;

    m_TimeDiv[ ch ] = timediv;
    setBeatLen( m_BeatLen );
}
//-----------------------------------------------------
// Procedure:   setDataAll
//-----------------------------------------------------
int Widget_EnvelopeEdit::getPos( int ch )
{
    return (int)( m_Clock[ ch ].fpos * 10000.0f );
}

//-----------------------------------------------------
// Procedure:   setDataAll
//-----------------------------------------------------
void Widget_EnvelopeEdit::setPos( int ch, int pos )
{
    m_Clock[ ch ].fpos = (float)pos / 10000.0f;
}

//-----------------------------------------------------
// Procedure:   setDataAll
//-----------------------------------------------------
void Widget_EnvelopeEdit::setDataAll( int *pint )
{
    int i, j, count = 0;

    if( !m_bInitialized )
        return;

    for( i = 0; i < MAX_ENVELOPE_CHANNELS; i++ )
    {
        for( j = 0; j < ENVELOPE_HANDLES; j++ )
        {
            m_HandleVal[ i ][ j ] = clamp( (float)pint[ count++ ] / 10000.0f, 0.0f, 1.0f );
        }
    }

    // recalc all lines
    recalcLine( -1, 0 );
}

//-----------------------------------------------------
// Procedure:   getDataAll
//-----------------------------------------------------
void Widget_EnvelopeEdit::getDataAll( int *pint )
{
    int i, j, count = 0;

    if( !m_bInitialized )
        return;

    for( i = 0; i < MAX_ENVELOPE_CHANNELS; i++ )
    {
        for( j = 0; j < ENVELOPE_HANDLES; j++ )
        {
            pint[ count++ ] = (int)( m_HandleVal[ i ][ j ] * 10000.0 );
        }
    }
}

//-----------------------------------------------------
// Procedure:   draw
//-----------------------------------------------------
int hdivs[ Widget_EnvelopeEdit::nRANGES ] = { 6, 11, 11, 21 };
void Widget_EnvelopeEdit::draw( NVGcontext *vg )
{
    int h;
    float x, y, divsize;
    float linewidth = 1.0;

    if( !m_bInitialized )
        return;

    // fill bg
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x-1, box.size.y-1 );
    //nvgFillColor(vg, nvgRGBA(40,40,40,255));
    nvgFillColor(vg, nvgRGBA(57, 10, 10,255));
    nvgFill(vg);


    nvgStrokeWidth( vg, linewidth );
    nvgStrokeColor( vg, nvgRGBA( 60, 60, 60, 255 ) );
    // draw bounding line
    /*nvgBeginPath( vg );
    nvgMoveTo( vg, 0, 0 );
    nvgLineTo( vg, box.size.x - 1, 0 );
    nvgLineTo( vg, box.size.x - 1, box.size.y - 1 );
    nvgLineTo( vg, 0, box.size.y - 1 );
    nvgClosePath( vg );
    nvgStroke( vg );*/

    x = 0.0;

    // draw vertical lines 
    for( h = 0; h < ENVELOPE_HANDLES; h++ )
    {
        nvgBeginPath(vg);
        nvgMoveTo( vg, x, 0 );
        nvgLineTo( vg, x, box.size.y - 1 );
        nvgStroke( vg );
        x += m_divw;
    }

    y = 0.0;

    divsize = box.size.y / (float)( hdivs[ m_Range[ m_currentChannel ] ] - 1 );

    // draw horizontal lines
    for( h = 0; h < hdivs[ m_Range[ m_currentChannel ] ]; h++ )
    {
        if( h == hdivs[ m_Range[ m_currentChannel ] ] / 2 && ( m_Range[ m_currentChannel ] == RANGE_n5to5 || m_Range[ m_currentChannel ] == RANGE_n10to10 ) )
            nvgStrokeColor( vg, nvgRGBA( 255, 255, 255, 255 ) );
        else
            nvgStrokeColor( vg, nvgRGBA( 80, 80, 80, 255 ) );

        nvgBeginPath(vg);
        nvgMoveTo( vg, 0, y );
        nvgLineTo( vg, box.size.x - 1, y );
        nvgStroke( vg );
        y += divsize;
    }

    if( m_bGateMode[ m_currentChannel ] )
    {
        // draw rects
        for( h = 0; h < ENVELOPE_DIVISIONS; h++ )
        {
            nvgBeginPath(vg);
            nvgRect( vg, h * m_divw, ( 1.0f - m_HandleVal[ m_currentChannel ][ h ] ) * box.size.y, m_divw, box.size.y * m_HandleVal[ m_currentChannel ][ h ] );
            nvgFillColor(vg, nvgRGBA( 157, 100, 100, 128 ) );
            nvgFill(vg);
        }
    }
    else
    {
        nvgStrokeColor( vg, nvgRGBA( 255, 255, 255, 255 ) );
        nvgBeginPath(vg);

        x = 0;
        nvgMoveTo( vg, x, ( 1.0f - m_HandleVal[ m_currentChannel ][ 0 ] ) * box.size.y );

        // draw lines
        for( h = 1; h < ENVELOPE_HANDLES; h++ )
        {
            x += m_divw;
            nvgLineTo( vg, x, ( 1.0f - m_HandleVal[ m_currentChannel ][ h ] ) * box.size.y );
        }

        nvgStroke( vg );

        x = 0;
        // draw handles
        for( h = 0; h < ENVELOPE_DIVISIONS + 1; h++ )
        {
            nvgBeginPath(vg);
            y = ( 1.0f - m_HandleVal[ m_currentChannel ][ h ] ) * box.size.y;
            nvgRect( vg, x - m_handleSizeD2, y - m_handleSizeD2, m_handleSize, m_handleSize );
            nvgFillColor(vg, nvgRGBA( m_HandleCol[ h ].Col[ 2 ], m_HandleCol[ h ].Col[ 1 ], m_HandleCol[ h ].Col[ 0 ],255 ) );
            nvgFill(vg);

             x += m_divw;
        }
    }

    // draw indicator line
    nvgStrokeColor( vg, nvgRGBA( 255, 255, 255, 80 ) );
    nvgBeginPath(vg);
    nvgMoveTo( vg, m_fIndicator[ m_currentChannel ] * box.size.x, 0 );
    nvgLineTo( vg, m_fIndicator[ m_currentChannel ] * box.size.x, box.size.y );
    nvgStroke( vg );
}

//-----------------------------------------------------
// Procedure:   onMouseDown
//-----------------------------------------------------
void Widget_EnvelopeEdit::onMouseDown( EventMouseDown &e )
{
    int index;

    m_Drag = -1;
    e.consumed = false;

    OpaqueWidget::onMouseDown( e );

    if( !m_bInitialized )
        return;

    if( e.button == 0 )
    {
        if( !m_bDraw )
            windowCursorLock();

        if( m_bGateMode[ m_currentChannel ] )
            m_Drag = clamp( ( e.pos.x / box.size.x ) * (float)ENVELOPE_DIVISIONS, 0.0f, (float)(ENVELOPE_DIVISIONS - 1) );
        else
            m_Drag = clamp( ( ( e.pos.x + (m_divw / 2.0f) ) / box.size.x ) * (float)ENVELOPE_DIVISIONS, 0.0f, (float)ENVELOPE_DIVISIONS );

        m_bDrag = true;
    }
    else if( e.button == 1 )
    {
        if( m_bGateMode[ m_currentChannel ] )
            index = clamp( ( e.pos.x / box.size.x ) * (float)ENVELOPE_DIVISIONS, 0.0f, (float)(ENVELOPE_DIVISIONS - 1) );
        else
            index = clamp( ( ( e.pos.x + (m_divw / 2.0f) ) / box.size.x ) * (float)ENVELOPE_DIVISIONS, 0.0f, (float)ENVELOPE_DIVISIONS );

        m_HandleVal[ m_currentChannel ][ index ] = 0.0f;
    }
}

//-----------------------------------------------------
// Procedure:   onDragStart
//-----------------------------------------------------
void Widget_EnvelopeEdit::onDragStart(EventDragStart &e)
{
    e.consumed = true;
    //windowCursorLock();
}

//-----------------------------------------------------
// Procedure:   onDragEnd
//-----------------------------------------------------
void Widget_EnvelopeEdit::onDragEnd(EventDragEnd &e)
{
    e.consumed = true;
     windowCursorUnlock();
     m_Drag = -1;
}

//-----------------------------------------------------
// Procedure:   onMouseMove
//-----------------------------------------------------
void Widget_EnvelopeEdit::onMouseMove( EventMouseMove &e )
{
    OpaqueWidget::onMouseMove( e );

    if( m_bDrag && m_bDraw )
    {
        m_Drawy = e.pos.y;

        if( m_bGateMode[ m_currentChannel ] )
            m_Drag = clamp( ( e.pos.x / box.size.x ) * (float)ENVELOPE_DIVISIONS, 0.0f, (float)(ENVELOPE_DIVISIONS - 1) );
        else
            m_Drag = clamp( ( ( e.pos.x + (m_divw / 2.0f) ) / box.size.x ) * (float)ENVELOPE_DIVISIONS, 0.0f, (float)ENVELOPE_DIVISIONS );
    }
}

//-----------------------------------------------------
// Procedure:   onDragMove
//-----------------------------------------------------
void Widget_EnvelopeEdit::onDragMove(EventDragMove &e)
{
    int h, i;
    float fband;
    float delta = 0.001f;

    e.consumed = true;

    if( !m_bInitialized || !m_bDrag )
        return;

    if( !m_bDraw )
    {
	    // Drag slower if Mod is held
	    if (windowIsModPressed())
		    delta = 0.00001f;

        m_HandleVal[ m_currentChannel ][ m_Drag ] -= delta * e.mouseRel.y;
        m_HandleVal[ m_currentChannel ][ m_Drag ] = clamp( m_HandleVal[ m_currentChannel ][ m_Drag ], 0.0f, 1.0f );

        if( m_pCallback )
            m_pCallback( m_pClass, getActualVal( m_currentChannel, m_HandleVal[ m_currentChannel ][ m_Drag ] ) );

        if( m_fband > 0.0001 )
        {
            fband = m_fband;

            for( h = -1; h > -4; h -- )
            {
                i = m_Drag + h;

                if( i < 0 )
                    break;

                m_HandleVal[ m_currentChannel ][ i ] -= (delta * e.mouseRel.y) * fband;
                m_HandleVal[ m_currentChannel ][ i ] = clamp( m_HandleVal[ m_currentChannel ][ i ], 0.0f, 1.0f );

                fband *= 0.6f;
            }

            fband = m_fband;

            for( h = 1; h < 4; h ++ )
            {
                i = m_Drag + h;

                if( i > ENVELOPE_DIVISIONS )
                    break;

                m_HandleVal[ m_currentChannel ][ i ] -= (delta * e.mouseRel.y) * fband;
                m_HandleVal[ m_currentChannel ][ i ] = clamp( m_HandleVal[ m_currentChannel ][ i ], 0.0f, 1.0f );

                fband *= 0.6f;
            }

            recalcLine( -1, m_Drag );
        }
        else
        {
            recalcLine( m_currentChannel, m_Drag );
        }
    }
    else
    {
        m_HandleVal[ m_currentChannel ][ m_Drag ] = 1.0 - ( m_Drawy / box.size.y );
        m_HandleVal[ m_currentChannel ][ m_Drag ] = clamp( m_HandleVal[ m_currentChannel ][ m_Drag ], 0.0f, 1.0f );

        if( m_pCallback )
            m_pCallback( m_pClass, getActualVal( m_currentChannel, m_HandleVal[ m_currentChannel ][ m_Drag ] ) );

        recalcLine( m_currentChannel, m_Drag );
    }
}

//-----------------------------------------------------
// Procedure:   setBeatLen
//-----------------------------------------------------
void Widget_EnvelopeEdit::setBeatLen( int len )
{
    m_BeatLen = len;
    m_bClkd = true;

    if( m_BeatLen <= 0 )
        return;

    for( int i = 0; i < MAX_ENVELOPE_CHANNELS; i++ )
    {
        switch( m_TimeDiv[ i ] )
        {
        case TIME_64th:
            m_Clock[ i ].syncInc = ( ( engineGetSampleRate() / (float)m_BeatLen ) * 16.0 ) / 16.0;
            break;
        case TIME_32nd:
            m_Clock[ i ].syncInc = ( ( engineGetSampleRate() / (float)m_BeatLen ) * 8.0 ) / 16.0;
            break;
        case TIME_16th:
            m_Clock[ i ].syncInc = ( ( engineGetSampleRate() / (float)m_BeatLen ) * 4.0 ) / 16.0;
            break;
        case TIME_8th:
            m_Clock[ i ].syncInc = ( ( engineGetSampleRate() / (float)m_BeatLen ) * 2.0 ) / 16.0;
            break;
        case TIME_4tr:
            m_Clock[ i ].syncInc = ( ( engineGetSampleRate() / (float)m_BeatLen ) * 1.0 ) / 16.0;
            break;
        case TIME_Bar:
            m_Clock[ i ].syncInc = ( ( engineGetSampleRate() / (float)m_BeatLen ) * 0.25 ) / 16.0;
            break;
        }
    }
}

//-----------------------------------------------------
// Procedure:   procStep
//-----------------------------------------------------
bool Widget_EnvelopeEdit::process_state( int ch, bool bTrig, bool bHold )
{
    switch( m_Clock[ ch ].state )
    {
    case STATE_RUN:
    case STATE_RUN_REV:

        if( bHold )
        {
            m_Clock[ ch ].prevstate = m_Clock[ ch ].state;
            m_Clock[ ch ].state = STATE_HOLD;
            break;
        }
        
        // run reverse
        if( m_Clock[ ch ].state == STATE_RUN_REV )
        {
            m_Clock[ ch ].fpos -= m_Clock[ ch ].syncInc;

            if( m_Clock[ ch ].fpos <= 0.0f )
            {
                switch( m_Mode[ ch ] )
                {
                case MODE_TWOSHOT:
                    m_Clock[ ch ].fpos = 0;
                    m_Clock[ ch ].state = STATE_WAIT_TRIG;
                    break;

                case MODE_PINGPONG:
                    m_Clock[ ch ].fpos = -m_Clock[ ch ].fpos;
                    m_Clock[ ch ].state = STATE_RUN;
                    break;

                case MODE_REVERSE:
                default:
                    m_Clock[ ch ].fpos += engineGetSampleRate();
                }
            }
        }
        // run forward
        else
        {
            m_Clock[ ch ].fpos += m_Clock[ ch ].syncInc;

            if( m_Clock[ ch ].fpos >= engineGetSampleRate() )
            {
                switch( m_Mode[ ch ] )
                {
                case MODE_ONESHOT:
                    m_Clock[ ch ].fpos = engineGetSampleRate() - 1.0f;;
                    m_Clock[ ch ].state = STATE_WAIT_TRIG;
                    break;
                case MODE_TWOSHOT:
                    m_Clock[ ch ].fpos = engineGetSampleRate() - 1.0f;
                    m_Clock[ ch ].state = STATE_WAIT_TRIG_REV;
                    break;
                case MODE_PINGPONG:
                    m_Clock[ ch ].fpos -= (m_Clock[ ch ].fpos - engineGetSampleRate()) * 2.0f;
                    m_Clock[ ch ].state = STATE_RUN_REV;
                    break;
                case MODE_LOOP:
                default:
                    m_Clock[ ch ].fpos -= engineGetSampleRate();
                }
            }
        }

        break;

    case STATE_WAIT_TRIG:
        if( bTrig )
        {
            m_Clock[ ch ].fpos = 0;
            m_Clock[ ch ].state = STATE_RUN;
        }
        break;
    case STATE_WAIT_TRIG_REV:
        if( bTrig )
        {
            m_Clock[ ch ].fpos = engineGetSampleRate();
            m_Clock[ ch ].state = STATE_RUN_REV;
        }
        break;

    case STATE_HOLD:
        if( !bHold )
        {
            m_Clock[ ch ].state = m_Clock[ ch ].prevstate;
            break;
        }
        break;
    }

    return true;
}

//-----------------------------------------------------
// Procedure:   procStep
//-----------------------------------------------------
float Widget_EnvelopeEdit::procStep( int ch, bool bTrig, bool bHold )
{
    int handle;

    if( ( m_bClkReset || bTrig ) && !bHold )
    {
        if( m_Mode[ ch ] == MODE_REVERSE )
            m_Clock[ ch ].fpos = engineGetSampleRate();
        else
            m_Clock[ ch ].fpos = 0;
    }

    process_state( ch, bTrig, bHold );

    m_fIndicator[ ch ] = m_Clock[ ch ].fpos / engineGetSampleRate();

    handle = (int)( m_Clock[ ch ].fpos / ( engineGetSampleRate() / (float)ENVELOPE_DIVISIONS ) );

    return valfromline( ch, handle, m_fIndicator[ ch ] * (float) box.size.x );
}