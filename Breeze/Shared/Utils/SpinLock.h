#ifndef __SHARED_UTILS_SPINLOCK_H__
#define __SHARED_UTILS_SPINLOCK_H__


#include "Lock.h"

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class SpinLock : public Lock
{
public:
	SpinLock(int count = SpinLock::TRY_COUNT);
	~SpinLock(void);

public:
	virtual void Acquire(void);
	virtual void Release(void);

protected:
	static const int TRY_COUNT;

protected:

#ifdef WIN32
	CRITICAL_SECTION lock_;
#else
	pthread_spinlock_t lock_;
#endif

};

#endif
