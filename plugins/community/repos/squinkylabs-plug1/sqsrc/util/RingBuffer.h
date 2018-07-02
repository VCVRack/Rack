#pragma once
#include <assert.h>

/**
 * A simple ring buffer.
 * Template arguments are for numeric type stored, and for size.
 * Not thread safe.
 * Guaranteed to be non-blocking. Adding or removing items will never
 * allocate or free memory.
 * Objects in RingBuffer are not owned by RingBuffer - they will not be destroyed.
 */
template <typename T, int SIZE>
class RingBuffer
{
public:
    RingBuffer()
    {
        for (int i = 0; i < SIZE; ++i) {
            memory[i] = 0;
        }
    }

    void push(T);
    T pop();
    bool full() const;
    bool empty() const;

private:
    T memory[SIZE];
    bool couldBeFull = false;           // full and empty are ambiguous, both are in--out
    int inIndex = 0;
    int outIndex = 0;

    /** Move up 'p' (a buffer index), wrap around if we hit the end
     * (this is the core of the circular ring buffer).
     */
    void advance(int &p);
};

template <typename T, int SIZE>
inline void RingBuffer<T, SIZE>::push(T value)
{
    assert(!full());
    memory[inIndex] = value;
    advance(inIndex);
    couldBeFull = true;
}


template <typename T, int SIZE>
inline T RingBuffer<T, SIZE>::pop()
{
    assert(!empty());
    T value = memory[outIndex];
    advance(outIndex);
    couldBeFull = false;
    return value;
}

template <typename T, int SIZE>
inline bool RingBuffer<T, SIZE>::full() const
{
    return (inIndex == outIndex) && couldBeFull;
}

template <typename T, int SIZE>
inline bool RingBuffer<T, SIZE>::empty() const
{
    return (inIndex == outIndex) && !couldBeFull;
}

template <typename T, int SIZE>
inline void RingBuffer<T, SIZE>::advance(int &p)
{
    if (++p >= SIZE) p = 0;
}



