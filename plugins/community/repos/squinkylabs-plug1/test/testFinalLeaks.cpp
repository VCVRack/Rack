
#include "LookupTable.h"
#include "ThreadSharedState.h"
#include "ThreadServer.h"
#include "FFTData.h"

#include "asserts.h"
extern int _numBiquads;

void testFinalLeaks()
{
    assertEQ(ThreadMessage::_dbgCount, 0);
    assertEQ(FFTDataReal::_count, 0);
    assertEQ(FFTDataCpx::_count, 0);
    assertEQ(ThreadSharedState::_dbgCount, 0);
    assertEQ(ThreadServer::_count, 0);
    assertEQ(_numLookupParams, 0);
    assertEQ(_numBiquads, 0)
}