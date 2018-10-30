//
// UpdateRing is similar in approach to double buffering 
// 
// UpdateRing provides a number (2) of objects so that the 
// module can communicate with the moduleWidget. Previously I
// have used mutexes to do this, but I would rather not use
// a blocking lock_guard in the audio thread.
//
// Double buffering does not offer the same guarantees, but if
// the code is kept tight, it should be good enough for practical purposes.
// 
// When the module has something to communicate, it asks for a pointer to the
// background object, It writes into this, and the swaps the buffers
//
// When the moduleWidget is checking for something to read, it asks for 
// a pointer to the foreground object, which it then can read.
//
// This process should minimize the risk of tearing artifacts in the process
//

template <class T>
struct UpdateRing {
	T items[2];
	unsigned int bufCount = 0;
	T* fg() { return items + (bufCount % 2); }	
	T* bg() { return items + ((bufCount + 1) % 2); }
	void swap() { bufCount++; };
};

