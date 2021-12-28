#pragma once
#include <common.hpp>
#include <pthread.h>


namespace rack {


/** Allows multiple "reader" threads to obtain a lock simultaneously, but only one "writer" thread.

This implementation is currently just a wrapper for pthreads, which works on Linux/Mac/.
This is available in C++17 as std::shared_mutex, but unfortunately we're using C++11.

Locking should be avoided in real-time audio threads.
*/
struct SharedMutex {
	pthread_rwlock_t rwlock;

	SharedMutex() {
		if (pthread_rwlock_init(&rwlock, NULL))
			throw Exception("pthread_rwlock_init failed");
	}
	~SharedMutex() {
		pthread_rwlock_destroy(&rwlock);
	}

	void lock() {
		if (pthread_rwlock_wrlock(&rwlock))
			throw Exception("pthread_rwlock_wrlock failed");
	}
	/** Returns whether the lock was acquired. */
	bool try_lock() {
		return pthread_rwlock_trywrlock(&rwlock) == 0;
	}
	void unlock() {
		if (pthread_rwlock_unlock(&rwlock))
			throw Exception("pthread_rwlock_unlock failed");
	}

	void lock_shared() {
		if (pthread_rwlock_rdlock(&rwlock))
			throw Exception("pthread_rwlock_rdlock failed");
	}
	/** Returns whether the lock was acquired. */
	bool try_lock_shared() {
		return pthread_rwlock_tryrdlock(&rwlock) == 0;
	}
	void unlock_shared() {
		unlock();
	}
};


template <class TMutex>
struct SharedLock {
	TMutex& m;

	SharedLock(TMutex& m) : m(m) {
		m.lock_shared();
	}
	~SharedLock() {
		m.unlock_shared();
	}
};


} // namespace rack
