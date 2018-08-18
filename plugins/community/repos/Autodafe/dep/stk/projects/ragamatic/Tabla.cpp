/***************************************************/
/*! \class Tabla
    \brief STK tabla drum class.

    This class implements a drum sampling synthesizer using FileWvIn
    objects and one-pole filters.  The drum rawwave files are sampled
    at 22050 Hz, but will be appropriately interpolated for other
    sample rates.  You can specify the maximum polyphony (maximum
    number of simultaneous voices) in Tabla.h.

    by Perry R. Cook and Gary P. Scavone, 1995--2017.
*/
/***************************************************/

#include "Tabla.h"
#include <cmath>
#include <sstream>

namespace stk {

Tabla :: Tabla( void ) : Instrmnt()
{
  // This counts the number of sounding voices.
  nSounding_ = 0;
  soundOrder_ = std::vector<int> (TABLA_POLYPHONY, -1);
  soundNumber_ = std::vector<int> (TABLA_POLYPHONY, -1);
}

Tabla :: ~Tabla( void )
{
}

static char tablaWaves[TABLA_NUMWAVES][16] =
  { "Drdak2.raw",
    "Drdak3.raw",
    "Drdak4.raw",
    "Drddak1.raw",
    "Drdee1.raw",
    "Drdee2.raw",
    "Drdoo1.raw",
    "Drdoo2.raw",
    "Drdoo3.raw",
    "Drjun1.raw",
    "Drjun2.raw",
    "DrDoi1.raw",
    "DrDoi2.raw",
    "DrTak1.raw",
    "DrTak2.raw"		    
  };

void Tabla :: noteOn( StkFloat instrument, StkFloat amplitude )
{
  if ( amplitude < 0.0 || amplitude > 1.0 ) {
    oStream_ << "Tabla::noteOn: amplitude parameter is out of bounds!";
    handleError( StkError::WARNING ); return;
  }

  int noteNumber = ( (int) instrument ) % 16;

  // If we already have a wave of this note number loaded, just reset
  // it.  Otherwise, look first for an unused wave or preempt the
  // oldest if already at maximum polyphony.
  int iWave;
  for ( iWave=0; iWave<TABLA_POLYPHONY; iWave++ ) {
    if ( soundNumber_[iWave] == noteNumber ) {
      if ( waves_[iWave].isFinished() ) {
        soundOrder_[iWave] = nSounding_;
        nSounding_++;
      }
      waves_[iWave].reset();
      filters_[iWave].setPole( 0.999 - (amplitude * 0.6) );
      filters_[iWave].setGain( amplitude );
      break;
    }
  }

  if ( iWave == TABLA_POLYPHONY ) { // This note number is not currently loaded.
    if ( nSounding_ < TABLA_POLYPHONY ) {
      for ( iWave=0; iWave<TABLA_POLYPHONY; iWave++ )
        if ( soundOrder_[iWave] < 0 ) break;
      nSounding_ += 1;
    }
    else {
      for ( iWave=0; iWave<TABLA_POLYPHONY; iWave++ )
        if ( soundOrder_[iWave] == 0 ) break;
      // Re-order the list.
      for ( int j=0; j<TABLA_POLYPHONY; j++ ) {
        if ( soundOrder_[j] > soundOrder_[iWave] )
          soundOrder_[j] -= 1;
      }
    }
    soundOrder_[iWave] = nSounding_ - 1;
    soundNumber_[iWave] = noteNumber;

    // Concatenate the rawwave path to the rawwave file
    waves_[iWave].openFile( (std::string("rawwaves/") + tablaWaves[ noteNumber ]).c_str(), true );
    if ( Stk::sampleRate() != 22050.0 )
      waves_[iWave].setRate( 22050.0 / Stk::sampleRate() );
    filters_[iWave].setPole( 0.999 - (amplitude * 0.6) );
    filters_[iWave].setGain( amplitude );
  }

  /*
#if defined(_STK_DEBUG_)
  oStream; errorStream << "Tabla::noteOn: number sounding = " << nSounding_ << '\n';
  for (int i=0; i<nSounding_; i++) oStream << soundNumber_[i] << "  ";
  oStream << '\n'; errorString_ = oStream.str();
  handleError( StkError::DEBUG_WARNING );
#endif
  */
}

void Tabla :: noteOff( StkFloat amplitude )
{
  // Set all sounding wave filter gains low.
  int i = 0;
  while ( i < nSounding_ ) filters_[i++].setGain( amplitude * 0.01 );
}

} // stk namespace
