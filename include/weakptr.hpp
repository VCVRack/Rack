#pragma once
#include <common.hpp>


namespace rack {


struct WeakHandle {
	void* ptr;
	size_t count = 0;
	WeakHandle(void* ptr) : ptr(ptr) {}
};


/** Base class for classes that allow `WeakPtr`s to be used. */
struct WeakBase {
	WeakHandle* weakHandle = nullptr;

	~WeakBase() {
		if (weakHandle) {
			weakHandle->ptr = nullptr;
		}
	}
	size_t getWeakCount() {
		if (!weakHandle)
			return 0;
		return weakHandle->count;
	}
};


/** A weak pointer to a subclass of WeakBase.
Weak pointers behave like raw pointers but can be accessed safely (returning nullptr) after the pointed object is deleted.

Example:

	Foo* foo = new Foo;
	WeakPtr<Foo> weakFoo = foo;
	weakFoo.get(); // returns foo
	delete foo;
	weakFoo.get(); // returns nullptr

Not thread safe.
In multithreaded environments, the object pointed to by the WeakPtr could be deleted at any time after obtaining its pointer from WeakPtr.
*/
template <typename T>
struct WeakPtr {
	WeakHandle* weakHandle = NULL;

	WeakPtr() {}
	WeakPtr(T* ptr) {
		set(ptr);
	}
	WeakPtr(const WeakPtr& other) {
		set(other.get());
	}
	WeakPtr& operator=(const WeakPtr& other) {
		set(other.get());
		return *this;
	}
	~WeakPtr() {
		set(nullptr);
	}
	void set(T* ptr) {
		// Release handle
		if (weakHandle) {
			// Decrement and check handle reference count
			if (--weakHandle->count == 0) {
				// Remove handle from object and delete it
				T* oldPtr = reinterpret_cast<T*>(weakHandle->ptr);
				if (oldPtr) {
					oldPtr->weakHandle = nullptr;
				}
				delete weakHandle;
			}
			weakHandle = nullptr;
		}
		// Obtain handle
		if (ptr) {
			if (!ptr->weakHandle) {
				// Create new handle for object
				ptr->weakHandle = new WeakHandle(ptr);
			}
			weakHandle = ptr->weakHandle;
			// Increment handle reference count
			weakHandle->count++;
		}
	}
	T* get() const {
		if (!weakHandle)
			return nullptr;
		return reinterpret_cast<T*>(weakHandle->ptr);
	}
	T* operator->() const {
		return get();
	}
	T& operator*() const {
		return *get();
	}
	operator T*() const {
		return get();
	}
	explicit operator bool() const {
		return get();
	}
};


} // namespace rack
