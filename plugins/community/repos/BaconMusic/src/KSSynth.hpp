// An implementation of the KSSYnth class in the ipython notebook in ../pynb/KarplusStrong.ipynb

#include <string>
#include <vector>
#include <math.h>
#include <cstdlib>

class KSSynth {
public:
  typedef enum InitPacket {
    RANDOM,
    SQUARE,
    SAW,
    NOISYSAW,
    SIN,
    SINCHIRP // if you add remember to fix the count below...
  } InitPacket;

  std::string initPacketName( InitPacket p )
  {
    switch(p)
      {
      case RANDOM: return "random";
      case SQUARE: return "square";
      case SAW: return "saw";
      case NOISYSAW: return "noisysaw";
      case SIN: return "sin";
      case SINCHIRP: return "sinchirp";
      }
    return "";
  }

  int numInitPackets()
  {
    return (int)SINCHIRP + 1;
  }

  typedef enum FilterType {
    WEIGHTED_ONE_SAMPLE
  } FilterType;

  std::string filterTypeName( FilterType p )
  {
    switch(p)
      {
      case WEIGHTED_ONE_SAMPLE: return "wgt 1samp";
      }
    return "";
  }

  int numFilterTypes()
  {
    return WEIGHTED_ONE_SAMPLE + 1;
  }

public:
  // here's my interior state
  InitPacket packet;
  FilterType filter;

  float filtParamA, filtParamB, filtParamC, filtAtten;

  float sumDelaySquared; // This is updated about 100 times a second, not per sample

  bool active;
  
  //private:
  float freq;
  int burstLen;
  float wfMin, wfMax;
  int sampleRate, sampleRateBy100;
  float filtAttenScaled;

  long pos;
  std::vector< float > delay;


public:

  KSSynth( float minv, float maxv, int sampleRateIn )
    :
    packet( RANDOM ),
    filter( WEIGHTED_ONE_SAMPLE ),

    filtParamA( 0.5f ),
    filtParamB( 0.0f ),
    filtParamC( 0.0f ),
    filtAtten( 3.0f ),

    active( false ),

    wfMin( minv ), wfMax( maxv ),
    sampleRate( sampleRateIn ),
    sampleRateBy100( (int)( sampleRate / 100 ) ),
    pos( 0 )
  {
    setFreq( 220 );
  }

  void setFreq( float f )
  {
    freq = f;
    burstLen = (int)( ( 1.0f  * sampleRate / freq + 0.5 ) * 2 );
    filtAttenScaled = filtAtten / 100 / ( freq / 440 );
    delay.resize( burstLen );
  }

  void trigger( float f )
  {
    // remember: Interally we work on the range [-1.0f, 1.0f] to match the python (but different from
    // the ChipSym which works on [ 0 1.0f ]
    active = true;
    setFreq( f );
    pos = 1;
    switch( packet )
      {
      case RANDOM:
        {
          for( int i=0; i<burstLen; ++i )
            {
              delay[ i ] = (float)rand() * 1.0 / RAND_MAX;
              delay[ i ] = delay[ i ] * 2.0 - 1.0;
            }
          break;
        }
      case SQUARE:
        {
          int xo = (int)burstLen / 2.0;
          for( int i=0; i<burstLen; ++i )
            {
              delay[ i ] = ( i <= xo ? 1.0 : -1.0 );
            }
          break;
        }
      case SAW:
        {
          for( int i=0; i<burstLen; ++i )
            {
              delay[ i ] = ( i * 2.0f / burstLen ) - 1.0;
            }
          break;
        }

      case NOISYSAW:
        {
          for( int i=0; i<burstLen; ++i )
            {
              delay[ i ] = ( i * 1.0f / burstLen ) - 0.5;
              delay[ i ] += (float)rand() * 1.0f / RAND_MAX - 0.5;
            }
          break;
        }

      case SIN:
        {
          float scale = 2.0 * M_PI / burstLen;
          for( int i=0; i<burstLen; ++i )
            {
              delay[ i ] = sin( i * scale );
            }
          break;
        }
      case SINCHIRP:
        {
          for( int i=0; i<burstLen; ++i )
            {
              float ls = 1.0f * i / burstLen;
              float lse = exp( ls * 2 ) * 3;
              delay[ i ] = sin( lse * 2 * M_PI );
            }
          break;
        }
      }
    updateSumDelaySquared();
  }

  void updateSumDelaySquared()
  {
    sumDelaySquared = 0;
    for( int i=0; i<burstLen; ++i )
      {
        sumDelaySquared += delay[ i ] * delay[ i ];
      }
    sumDelaySquared /= burstLen;

  }
  
  float step()
  {
    if( ! active ) return 0;
    
    int dpos = pos % burstLen;
    int dpnext = ( pos + 1 ) % burstLen;
    int dpfill = ( pos - 1 ) % burstLen;
    if( pos % sampleRateBy100 == 0 )
      {
        updateSumDelaySquared();
        if( sumDelaySquared < 1e-7 ) // think about this threshold some
          active = false;
      }
    pos++;


    float filtval = 0.5;
    switch( filter )
      {
      case WEIGHTED_ONE_SAMPLE:
        float ftw = filtParamA;
        float fta = filtAttenScaled;
        filtval = ( ftw * delay[ dpos ] + ( 1.0 - ftw ) * delay[ dpnext ] ) * ( 1.0 - fta );
        break;
      }
    delay[ dpfill ] = filtval;
    return ( ( filtval + 1.0 ) * 0.5 ) * ( wfMax - wfMin ) + wfMin;
  }
  
};
