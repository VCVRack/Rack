#include "../src/ChipSym.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <cassert>

int main( int argc, char ** argv )
{
  std::cout << "Testing chip periods" << std::endl;
  ChipSym::NESNoise noise( 0.0f, 1.0f, 44100 );
  noise.setPeriod( 2 );
  noise.setModeFlag( true );

  for( int i=0; i<5; ++i )
    {
      // So lets start by testing if 31 gives us 31 and 93 gives us 93
      int ct = 0;
      noise.setShortLength( ChipSym::NESNoise::SHORT_31 );
      unsigned short target = noise.getRegister();
      do { ct ++; noise.advanceRegister(); } while( noise.getRegister() != target );
      assert( ct == 31 );

      noise.setShortLength( ChipSym::NESNoise::SHORT_93 );
      target = noise.getRegister();
      ct = 0;
      do { ct ++; noise.advanceRegister(); } while( noise.getRegister() != target && ct < 100 );
      assert( ct == 93 );
    }
  
  std::map< int, std::pair< unsigned short, int > > resultMap;
  std::map< unsigned short, int > sequenceMap;
  int sequenceID = 0;

  
  for( unsigned short iRegister=1; iRegister < 0x7FFF; iRegister ++ )
    {
      bool newSeq = false;
      noise.setRegister( iRegister );
      // step off my initial point into the sequence
      while( noise.getRegister() == iRegister ) noise.advanceRegister();
      
      unsigned short target = noise.getRegister();
      bool gotR = false;
      int stepC = 0;
      
      if( sequenceMap.find( noise.getRegister() ) == sequenceMap.end() ) {
        sequenceID ++;
        newSeq = true;
      }
      
      while( ! gotR )
        {
          ++stepC;
          if( newSeq )
            sequenceMap[ noise.getRegister() ] = sequenceID;
                    
          noise.advanceRegister();
          if( noise.getRegister() == target )
            {
              if( resultMap.find( stepC ) == resultMap.end() )
                {
                  resultMap[ stepC ] = std::pair< unsigned short, int >( target, 1 );
                }
              else
                resultMap[ stepC ].second ++;
              
              gotR = true;
            }
        }
    }

  std::cout << "Unique sequence count is " << sequenceID << " " << sequenceMap.size() << "\n";

  // Invert sequenceMap
  std::map< int, std::set< unsigned short > > inverseSequenceMap;
  for( auto smapKey = sequenceMap.begin(); smapKey != sequenceMap.end(); ++smapKey )
    {
      inverseSequenceMap[ smapKey->second ].insert( smapKey->first );
    }

  std::map< int, int > lenMap;
  lenMap[ 93 ] = 0;
  lenMap[ 31 ] = 0;
  for( auto imapKey = inverseSequenceMap.begin(); imapKey != inverseSequenceMap.end(); ++imapKey )
    {
        lenMap[ imapKey->second.size() ] ++;
    }

  std::cout << "Count[ 93 ] = " << lenMap[ 93 ] << "\nCount[ 31 ] = " << lenMap[ 31 ] << "\n";
  
  for( auto imapKey = inverseSequenceMap.begin(); imapKey != inverseSequenceMap.end(); ++imapKey )
    {
      std::cout << "Seq: " << std::setw( 4 ) << std::setfill( ' ' ) << std::setbase( 10 ) << imapKey->first << " Len = " << imapKey->second.size() << " ";
      int ct = 0;
      for( auto secKey = imapKey->second.begin(); secKey != imapKey->second.end() && ct < 8; ++secKey, ++ct )
        {
          std::cout << " 0x" << std::setw( 4 ) << std::setbase( 16 ) << std::setfill( '0' ) << *secKey;
        }
      std::cout << "\n";
    }

  // Finally dump the c structure
  std::cout << "unsigned short start_for_93s[] = {\n    ";
  int ct = 0;
  for( auto imapKey = inverseSequenceMap.begin(); imapKey != inverseSequenceMap.end(); ++imapKey, ++ct )
    {
      if( imapKey->second.size() == 93 )
        std::cout << std::setbase( 10 ) <<  *( imapKey->second.begin() ) << ", ";
      if( (ct+1) % 15 == 0 )
        std::cout << "\n    ";
    }
  std::cout << " 0 };\n";
   
  
}
