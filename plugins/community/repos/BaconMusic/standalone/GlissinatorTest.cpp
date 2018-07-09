#include "standalone_helpers.hpp"
#include "../src/Glissinator.hpp"
#include <iostream>

int main( int argch, char **argv )
{
  std::cout << "Test Glissinator\n";
  typedef Glissinator< StandaloneModule > G;
  

  for( int io = 0; io <= 1; ++io )
  {
    G g;
    
    g.params[ G::GLISS_TIME ].value = 0.1;
    g.inputs[ G::SOURCE_INPUT ].value = 1.00 + io;
    g.inputs[ G::SOURCE_INPUT ].active = true;
    g.outputs[ G::SLID_OUTPUT ].active = true;
    
    G::results_t ov;
    g.multiStep( 100, ov );
    
    
    g.inputs[ G::SOURCE_INPUT ].value = 2.00 - io;
    g.multiStep( engineGetSampleRate() * 0.15, ov );

    // So this should be monotonically increasing.
    auto hd = ov.begin() + 1, pv = ov.begin();
    while( hd != ov.end() )
      {
        if( io == 0 )
          assert( (*hd)[ G::SLID_OUTPUT ].value >= (*pv)[ G::SLID_OUTPUT ].value );
        else
          assert( (*hd)[ G::SLID_OUTPUT ].value <= (*pv)[ G::SLID_OUTPUT ].value );
        
        assert( (*hd)[ G::SLID_OUTPUT ].value >= 1 );
        assert( (*hd)[ G::SLID_OUTPUT ].value <= 2 );
        ++hd;
        ++pv;
      }
    std::cout << "PASSED: Simple case is monotonic " << 1 + io << " -> " << 2 - io << "\n";
  }

  // The turnadound-half-way-through test
  for( int io = 0; io <= 1; ++io )
  {
    G g;
    
    g.params[ G::GLISS_TIME ].value = 0.1;
    g.inputs[ G::SOURCE_INPUT ].value = 1.00 + io;
    g.inputs[ G::SOURCE_INPUT ].active = true;
    g.outputs[ G::SLID_OUTPUT ].active = true;
    
    G::results_t ov;
    g.multiStep( 100, ov );
    
    
    g.inputs[ G::SOURCE_INPUT ].value = 2.00 - io;
    g.multiStep( engineGetSampleRate() * 0.07, ov );

    float maxO = g.outputs[ G::SLID_OUTPUT ].value;
    g.inputs[ G::SOURCE_INPUT ].value = 1.00 + io;
    g.multiStep( engineGetSampleRate() * 0.07, ov );
        
    // So this should no longer be monotonic strictly but should monotone up to max
    // and then down from
    bool goingUp = (io == 0)?true:false;
    bool hitMax = false;
    auto hd = ov.begin() + 1, pv = ov.begin();
    while( hd != ov.end() )
      {
        if( goingUp )
          assert( (*hd)[ G::SLID_OUTPUT ].value >= (*pv)[ G::SLID_OUTPUT ].value );
        else
          assert( (*hd)[ G::SLID_OUTPUT ].value <= (*pv)[ G::SLID_OUTPUT ].value );

        if( io == 0 )
          {
            assert( (*hd)[ G::SLID_OUTPUT ].value >= 1 );
            assert( (*hd)[ G::SLID_OUTPUT ].value <= maxO );
          }
        else
          {
            assert( (*hd)[ G::SLID_OUTPUT ].value <= 2 );
            assert( (*hd)[ G::SLID_OUTPUT ].value >= maxO );
          }

        if( (*hd)[G::SLID_OUTPUT].value == maxO && ! hitMax ) { goingUp = ! goingUp; hitMax = true; } // max can repeat
        
        ++hd;
        ++pv;
      }
    std::cout << "PASSED: Turnaround case is bi-monotonic " << 1 + io << " -> " << 2 - io << "\n";
  }

    
  {
    G g;
    // OK so now lets test that gliss time bug. If we reset the gliss time most of the way through
    // a gliss, the 0.6.1 version runs away.
    g.params[ G::GLISS_TIME ].value = 0.1;
    g.inputs[ G::SOURCE_INPUT ].value = 1.00;
    g.inputs[ G::SOURCE_INPUT ].active = true;
    g.outputs[ G::SLID_OUTPUT ].active = true;

    G::results_t ov;
    g.multiStep( 100, ov );
    
    g.inputs[ G::SOURCE_INPUT ].value = 2.00 ;
    g.multiStep( engineGetSampleRate() * 0.07, ov );

    g.params[ G::GLISS_TIME ].value = 0;

    g.multiStep( 1, ov );
    
    auto hd = ov.begin() + 1, pv = ov.begin();
    while( hd != ov.end() )
      {
        assert( (*hd)[ G::SLID_OUTPUT ].value >= (*pv)[ G::SLID_OUTPUT ].value );
        assert( (*hd)[ G::SLID_OUTPUT ].value >= 1 );
        assert( (*hd)[ G::SLID_OUTPUT ].value <= 2 );
        ++hd;
        ++pv;
      }
    std::cout << "PASSED: Jump case is still monotonic and bounded\n";
  }

}
