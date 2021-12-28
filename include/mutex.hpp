#pragma once
#include <common.hpp>
#include <pthread.h>


namespace rack {


/** Allows multiple "reader" threads to obtain a lock simultaneously, but only one "writer" thread.
This implementation is currently just a wrapper for pthreads, which works on Linux/Mac/.
This is available in C++17 as std::shared_mutex, but unfortunately we're using C++11.
*/
struct ReadWriteMutex {
	pthread_rwlock_t rwlock;

	ReadWriteMutex() {
		if (pthread_rwlock_init(&rwlock, NULL))
			throw Exception("pthread_rwlock_init failed");
	}
	~ReadWriteMutex() {
		pthread_rwlock_destroy(&rwlock);
	}

	void lockReader() {
		if (pthread_rwlock_rdlock(&rwlock))
			throw Exception("pthread_rwlock_rdlock failed");
	}
	/** Returns whether the lock was acquired. */
	bool tryLockReader() {
		return pthread_rwlock_tryrdlock(&rwlock) == 0;
	}
	void unlockReader() {
		if (pthread_rwlock_unlock(&rwlock))
			throw Exception("pthread_rwlock_unlock failed");
	}

	void lockWriter() {
		if (pthread_rwlock_wrlock(&rwlock))
			throw Exception("pthread_rwlock_wrlock failed");
	}
	/** Returns whether the lock was acquired. */
	bool tryLockWriter() {
		return pthread_rwlock_trywrlock(&rwlock) == 0;
	}
	void unlockWriter() {
		if (pthread_rwlock_unlock(&rwlock))
			throw Exception("pthread_rwlock_unlock failed");
	}
};


struct ReadLock {
	ReadWriteMutex& m;

	ReadLock(ReadWriteMutex& m) : m(m) {
		m.lockReader();
	}
	~ReadLock() {
		m.unlockReader();
	}
};


struct WriteLock {
	ReadWriteMutex& m;

	WriteLock(ReadWriteMutex& m) : m(m) {
		m.lockWriter();
	}
	~WriteLock() {
		m.unlockWriter();
	}
};


} // namespace rack
