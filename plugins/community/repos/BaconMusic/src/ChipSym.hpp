/*
** All my chip simulator noobits. But remember this is just me screwing around.
*/

#include <math.h>
#include <iostream>
#include <vector>

namespace ChipSym
{
  class CPUStepper
  {
  private:
    int sampleRateInHz;
    double chipFrequencyInMHZ;

    unsigned int ticksPerSample;
    double tickFractionPerSample;
    double accruedTickFraction;
    
  public:
    CPUStepper( unsigned int _sampleRateInHz, double _chipFrequencyInMHZ )
      : sampleRateInHz( _sampleRateInHz ), chipFrequencyInMHZ( _chipFrequencyInMHZ ), accruedTickFraction( 0 )
    {
      double tpsD = chipFrequencyInMHZ * 1000000 / sampleRateInHz;
      double tpsdi;
      tickFractionPerSample = modf( tpsD, &tpsdi );
      ticksPerSample = (unsigned int)tpsdi;
    }

    /* 
    ** Take one step and tell me how many CPU ticks I would have seen if I am sampling at sampleRate.
    ** This won't be a constant of course since sometimes we get an extra to catch up
    */
    
    unsigned int nextStepCPUTicks()
    {
      accruedTickFraction += tickFractionPerSample;
      if( accruedTickFraction > 1 )
        {
          accruedTickFraction -= 1;
          return ticksPerSample + 1;
        }

      return ticksPerSample;
    }
  };

  static double NESNTSCCPURate = 1.789773;


  class NESBase
  {
  protected:
    int digWavelength; // this is the 2^11 which sets frequency time
    int t, currPos;

    float wfMin, wfMax, wfMinToMax;

  public:
    NESBase( float imin, float imax )
      :
      wfMin( imin ), wfMax( imax )
    {
      digWavelength = 1 << 7; // Callibrate this later
      t = digWavelength;
      currPos = 0;
      
      wfMinToMax = wfMax - wfMin;
    }

    void setDigWavelength( int df ) // 0 -> 2^11
    {
      digWavelength = df;
    }
  };
  
  class NESTriangle : public NESBase // http://wiki.nesdev.com/w/index.php/APU_Triangle
  {
  protected:
    float waveForm[ 32 ];
    CPUStepper cpu;


  public:
    NESTriangle( float imin, float imax, unsigned int sampleRate )
      :
      NESBase( imin, imax ), cpu( sampleRate, NESNTSCCPURate )
    {
      for( int i=0; i<16; ++i ) {
        waveForm[ 15 - i ] = i / 15.0f;
        waveForm[ 16 + i ] = i / 15.0f;
      }
    }

    float step()
    {
      int ticks = cpu.nextStepCPUTicks();
      t -= ticks;
      if( t < 0 )
        {
          currPos ++;
          t += digWavelength;
          if( currPos >= 32 ) currPos = 0;
        }
      
      return waveForm[ currPos ] * wfMinToMax + wfMin;
    }

    void setWavelengthInSeconds( float seconds )
    {
      setDigWavelength( (unsigned int)( seconds * NESNTSCCPURate * 1000 * 1000 / 32 ) );
    }
  };

  class NESArbitraryWaveform : public NESTriangle {
  public:
    NESArbitraryWaveform( float imin, float imax, unsigned int sampleRate ) : NESTriangle( imin, imax, sampleRate ) { }

    void setWaveformPoint( unsigned int pos,  // 0->31
                           unsigned int val ) // 0->15
    {
      waveForm[ pos  ] = val / 15.0f;
    }

    unsigned int getWaveformPoint( unsigned int pos ) { return waveForm[ pos ] * 15.0f; }
  };

  class NESPulse : public NESBase // http://wiki.nesdev.com/w/index.php/APU_Pulse
  {
  private:
    int dutyCycle;
    float **waveForms;
    int nDutyCycles;
    int wfLength;
    CPUStepper cpu;

    
  public:
    NESPulse( float imin, float imax, int sampleRate )
      :
      NESBase( imin, imax ), cpu( sampleRate, NESNTSCCPURate / 2 )
    {
      wfLength = 8;
      nDutyCycles = 4;
      dutyCycle = 1;
      
      waveForms = new float*[ 4 ];
      for( int i=0; i<nDutyCycles; ++i )
        {
          waveForms[ i ] = new float[ wfLength ];
          for (int j=0; j<wfLength; ++j ) waveForms[ i ][ j ] = ( i == nDutyCycles - 1 ) ? 1 : 0;

          // Really, read that website for this stuff.
          switch( i )
            {
            case 0:
              waveForms[ i ][ 1 ] = 1;
              break;
            case 1:
              waveForms[ i ][ 1 ] = 1;
              waveForms[ i ][ 2 ] = 1;
              break;
            case 2:
              waveForms[ i ][ 1 ] = 1;
              waveForms[ i ][ 2 ] = 1;
              waveForms[ i ][ 3 ] = 1;
              waveForms[ i ][ 4 ] = 1;
              break;
            case 3:
              waveForms[ i ][ 1 ] = 0;
              waveForms[ i ][ 2 ] = 0;
              break;
            }
        }

    }

    void setWavelengthInSeconds( float seconds )
    {
      setDigWavelength( (unsigned int)( seconds * NESNTSCCPURate * 1000 * 1000 / 2.0 / 8.0) );
    }

    void setDutyCycle( int dc )
    {
      dutyCycle = dc;
    }
    
    float step()
    {
      int ticks = cpu.nextStepCPUTicks();
      t -= ticks;
      if( t < 0 )
        {
          currPos ++;
          t += digWavelength;
          if( currPos >= wfLength  ) currPos = 0;
        }
      
      return waveForms[ dutyCycle ][ currPos ] * wfMinToMax + wfMin;
    }
  };

  class NESNoise : public NESBase
  {
  public:
    typedef enum ShortPeriods
      {
        SHORT_31,
        SHORT_93
      } ShortPeriods;

  private:
    CPUStepper cpu;
    unsigned short shiftRegister;
    unsigned short currentOutput;
    unsigned short xorBit;

    unsigned short curr93key;
    std::vector< unsigned short > starts_for_93s;
    ShortPeriods currShortPeriods;
    
  public:

    NESNoise( float imin, float imax, int sampleRate )
      :
      NESBase( imin, imax ), cpu( sampleRate, NESNTSCCPURate / 2 )
    {
      setPeriod( 8 );
      shiftRegister = 0x07;
      currentOutput = shiftRegister & 1;
      xorBit = 1;
      curr93key = 17;
      init93();
      currShortPeriods = SHORT_93;
    }

    void init93()
    {
      // To generate this, see ../standalone/chipNoisePeriod.cpp
      unsigned short calc_start_for_93s[] = {
        1, 3, 5, 7, 11, 13, 15, 17, 19, 21, 23, 25, 29, 31, 33, 
        35, 37, 39, 41, 43, 47, 49, 51, 53, 55, 57, 59, 61, 66, 68, 
        70, 74, 76, 78, 80, 84, 86, 88, 90, 92, 94, 96, 98, 102, 104, 
        106, 108, 110, 112, 114, 116, 120, 122, 124, 126, 129, 131, 133, 135, 139, 
        141, 143, 145, 147, 149, 151, 153, 157, 159, 161, 163, 165, 167, 169, 171, 
        175, 177, 179, 181, 183, 185, 187, 189, 194, 196, 198, 200, 202, 204, 206, 
        208, 212, 214, 218, 220, 222, 224, 226, 230, 232, 234, 236, 238, 240, 242, 
        244, 248, 250, 252, 254, 259, 263, 267, 271, 273, 275, 277, 279, 281, 285, 
        287, 291, 295, 299, 303, 305, 307, 309, 311, 313, 315, 317, 322, 326, 330, 
        334, 336, 340, 342, 344, 346, 348, 350, 354, 358, 362, 366, 368, 370, 372, 
        376, 378, 380, 382, 387, 389, 395, 397, 399, 401, 403, 405, 407, 409, 413, 
        415, 417, 419, 421, 423, 425, 427, 431, 435, 437, 443, 445, 450, 452, 454, 
        456, 458, 460, 464, 468, 470, 472, 474, 478, 482, 486, 488, 490, 492, 494, 
        498, 500, 506, 535, 543, 547, 551, 555, 559, 563, 565, 571, 573, 598, 606, 
        610, 614, 618, 622, 626, 628, 634, 636, 645, 647, 653, 655, 661, 663, 669, 
        671, 673, 677, 679, 681, 687, 689, 691, 693, 699, 701, 710, 716, 718, 724, 
        726, 732, 734, 742, 744, 750, 752, 756, 760, 762, 764, 775, 779, 785, 787, 
        789, 793, 797, 799, 803, 811, 815, 819, 821, 823, 827, 829, 834, 838, 842, 
        852, 858, 860, 862, 870, 874, 880, 882, 884, 888, 892, 894, 901, 907, 913, 
        915, 919, 927, 931, 933, 935, 947, 962, 970, 982, 988, 990, 994, 1000, 1004, 
        1012, 1018, 1057, 1059, 1061, 1063, 1065, 1067, 1075, 1077, 1079, 1081, 1083, 1100, 1110, 
        1116, 1130, 1132, 1134, 1138, 1144, 1185, 1187, 1189, 1191, 1195, 1201, 1209, 1211, 1228, 
        1238, 1244, 1264, 1274, 1293, 1311, 1313, 1335, 1354, 1392, 1415, 1419, 1425, 1455, 1467, 
        1510, 1524, 1569, 1620, 1638, 1671, 1868, 0 };
      unsigned short* curr = calc_start_for_93s;
      while( *curr != 0 )
        {
          starts_for_93s.push_back( *curr );
          ++curr;
        }
    }

    void set93Key( int sp )
    {
      if( sp != curr93key )
        {
          setRegister( starts_for_93s[ sp ] );
        }
      curr93key = sp;
    }
    void setModeFlag( bool mf )
    {
      if( mf )
        {
          xorBit = 6;

          // set register based on cached shortlen
          setShortLength( currShortPeriods );
        }
      else xorBit = 1;
    }

    
    void setShortLength( ShortPeriods p )
    {
      currShortPeriods = p;
      if( p == SHORT_31 )
        {
          setRegister( 0x0737 );
        }
      else
        {
          // A little state so we don't always land on the same one
          setRegister( starts_for_93s[ curr93key ] );
        }
    }

    void setPeriod( unsigned int c ) // 0 - 15
    {
      if( c > 15 ) c = 8;
      switch( c ) {
      case 0:
        digWavelength = 4; break;
      case 1:
        digWavelength = 8; break;
      case 2:
        digWavelength = 16; break;
      case 3:
        digWavelength = 32; break;
      case 4:
        digWavelength = 64; break;
      case 5:
        digWavelength = 96; break;
      case 6:
        digWavelength = 128; break;
      case 7:
        digWavelength = 160; break;
      case 8:
        digWavelength = 202; break;
      case 9:
        digWavelength = 254; break;
      case 10:
        digWavelength = 380; break;
      case 11:
        digWavelength = 508; break;
      case 12:
        digWavelength = 762; break;
      case 13:
        digWavelength = 1016; break;
      case 14:
        digWavelength = 2034; break;
      case 15:
        digWavelength = 4068; break;
      }
    }

    void advanceRegister()
    {
      // Do the LFSR Shift
      unsigned short bit  = ((shiftRegister >> 0) ^ (shiftRegister >> xorBit)) & 1;
      shiftRegister =  (shiftRegister >> 1) | (bit << 14); // thanks https://github.com/baconpaul/BaconPlugs/issues/6
    }
    
    float step()
    {
      int ticks = cpu.nextStepCPUTicks();
      t -= ticks;
      if( t < 0 )
        {
          t += digWavelength;
          advanceRegister();
    
          currentOutput = shiftRegister & 1;
        }
      
      return currentOutput * wfMinToMax + wfMin;

    }

    void setRegister( unsigned short r ) { shiftRegister = r; }
    unsigned short getRegister() { return shiftRegister; }
  };

#if 0
  class LFSRGeneralImpl
  {
  public:
    typedef std::bitset< 24 > bits;
  private:
  public:
    void setActivetBits( size_t aBits ) // < 24 please
    {
    }
    void setTapsAsInt( unsigned int taps ) // so 1 << 16 & 1 << 14 & 1 << 7 type thing
    {
    }
  };
#endif
  
};
