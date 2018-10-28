#pragma once

#include <vector>
#include <memory>

#include "RingBuffer.h"

/**
 * A very specialized container. Made for holding one free
 * work buffers, and making sure they are destroyed.
 *
 * At construction time, objects are created to fill the pool.
 * pop the objects to get one, push back to return when done.
 *
 * Destructor will delete all the objects, even if they are not in the pool
 * at the time.
 *
 * Note that unlike RingBuffer, ManagePool manages T*, not T.
 *
 * All public functions are no-blocking, so may be called from the audio thread
 * without danger. Of course the constructor and destructor are exceptions - they may block.
 */
template <typename T, int SIZE>
class ManagedPool
{
public:
    ManagedPool();


    /** Accessors from RingBuffer
     */
    void push(T*);
    T* pop();
    bool full() const;
    bool empty() const;
private:
    /**
     * this ring buffer is where the raw T* are kept.
     * client pops and pushes here
     */
    SqRingBuffer<T*, SIZE> ringBuffer;
    std::vector< std::unique_ptr<T>> lifetimeManager;
};

template <typename T, int SIZE>
inline ManagedPool<T, SIZE>::ManagedPool()
{
    // Manufacture the items here
    for (int i = 0; i < SIZE; ++i) {
        T * item = new T();
        ringBuffer.push(item);          // put the raw pointer in ring buffer for client to use.

        // Also add managed pointers to delete the objects on destroy.
        lifetimeManager.push_back(std::unique_ptr<T>(item));
    }
}

template <typename T, int SIZE>
inline void ManagedPool<T, SIZE>::push(T* value)
{
    ringBuffer.push(value);
}


template <typename T, int SIZE>
inline T* ManagedPool<T, SIZE>::pop()
{
    return ringBuffer.pop();
}

template <typename T, int SIZE>
inline bool ManagedPool<T, SIZE>::full() const
{
    return ringBuffer.full();
}

template <typename T, int SIZE>
inline bool ManagedPool<T, SIZE>::empty() const
{
    return ringBuffer.empty();
}


