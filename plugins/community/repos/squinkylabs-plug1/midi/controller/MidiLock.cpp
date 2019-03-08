
#include "MidiLock.h"

#include <assert.h>

MidiLock::MidiLock()
{
    theLock = false;
    editorLockLevel = 0;
    editorDidLock = false;
}

MidiLockPtr MidiLock::make()
{
    return std::make_shared<MidiLock>();
}

void MidiLock::editorLock()
{
    if (editorLockLevel == 0) {

        // poll to take lock
        for (bool done = false; !done; ) {
            done = tryLock();
        }
    }
    ++editorLockLevel;
    editorDidLock = true;
    const int l = editorLockLevel;
}
void MidiLock::editorUnlock()
{
    const int l = editorLockLevel;
    if (--editorLockLevel == 0) {
        theLock = false;
    }
}

bool MidiLock::playerTryLock()
{
    // try once to take lock
    return tryLock();
}

void MidiLock::playerUnlock()
{
    assert(locked());
    theLock = false;
}

bool MidiLock::tryLock()
{
    bool expected = false;
    bool desired = true;
    bool ret = theLock.compare_exchange_weak(expected, desired);
    return ret;
}

bool MidiLock::locked() const
{
    return theLock;
}

bool MidiLock::dataModelDirty() 
{
    bool ret = editorDidLock;
    editorDidLock = false;
    return ret;
}

/***********************************************************************/


MidiLocker::MidiLocker(MidiLockPtr l) : lock(l)
{
    lock->editorLock();
}


MidiLocker::~MidiLocker()
{
    lock->editorUnlock();
}