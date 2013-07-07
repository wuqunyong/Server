#ifndef __SHARED_UTILS_LOCK_H__
#define __SHARED_UTILS_LOCK_H__

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class Lock
{
public:
	virtual void Acquire(void) = 0;
	virtual void Release(void) = 0;

public:
	virtual ~Lock(void) { }
};

#endif 