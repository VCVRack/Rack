/***************************************************/
/*! \class Drone
    \brief STK "drone" plucked string model.

    This class implements a simple plucked string
    physical model based on the Karplus-Strong
    algorithm.

    This is a digital waveguide model, making its
    use possibly subject to patents held by
    Stanford University, Yamaha, and others.
    There exist at least two patents, assigned to
    Stanford, bearing the names of Karplus and/or
    Strong.

    by Perry R. Cook and Gary P. Scavone, 1995--2017.
*/
/***************************************************/

#include "Drone.h"
#include <sstream>

namespace stk {

Drone :: Drone( StkFloat lowestFrequency )
{
  if ( lowestFrequency <= 0.0 ) {
    oStream_ << "Drone::Drone: argument is less than or equal to zero!";
    handleError( StkError::FUNCTION_ARGUMENT );
  }

  unsigned long delays = (unsigned long) ( Stk::sampleRate() / lowestFrequency );
  delayLine_.setMaximumDelay( delays + 1 );

  this->setFrequency( 220.0 );
  envelope_.setAllTimes( 2.0, 0.5, 0.0, 0.5 );
  this->clear();
}

Drone :: ~Drone( void )
{
}

void Drone :: clear( void )
{
  delayLine_.clear();
  loopFilter_.clear();
}

void Drone :: setFrequency( StkFloat frequency )
{
#if defined(_STK_DEBUG_)
  if ( frequency <= 0.0 ) {
    oStream_ << "Drone::setFrequency: argument is less than or equal to zero!";
    handleError( StkError::WARNING ); return;
  }
#endif

  // Delay = length - approximate filter delay.
  StkFloat delay = (Stk::sampleRate() / frequency) - 0.5;
  delayLine_.setDelay( delay );

  loopGain_ = 0.997 + (frequency * 0.000002);
  if ( loopGain_ >= 1.0 ) loopGain_ = 0.99999;
}

void Drone :: pluck( StkFloat amplitude )
{
  envelope_.keyOn();
}

void Drone :: noteOn( StkFloat frequency, StkFloat amplitude )
{
  this->setFrequency( frequency );
  this->pluck( amplitude );
}

void Drone :: noteOff( StkFloat amplitude )
{
  if ( amplitude < 0.0 || amplitude > 1.0 ) {
    oStream_ << "Plucked::noteOff: amplitude is out of range!";
    handleError( StkError::WARNING ); return;
  }

  loopGain_ = 1.0 - amplitude;
}

} // stk namespace
