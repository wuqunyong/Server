#include "SpinLock.h"

const int SpinLock::TRY_COUNT = 5;

SpinLock::SpinLock(int count)
{
#ifdef WIN32 
	::InitializeCriticalSectionAndSpinCount(&lock_, count);
#else
	::pthread_spin_init(&lock_, count);
#endif
}

SpinLock::~SpinLock(void)
{
#ifdef WIN32 
	::DeleteCriticalSection(&lock_);
#else
	::pthread_spin_destroy(&lock_);
#endif
}

void SpinLock::Acquire(void)
{
#ifdef WIN32 
	::EnterCriticalSection(&lock_);
#else
	::pthread_spin_lock(&lock_);
#endif
}

void SpinLock::Release(void)
{
#ifdef WIN32 
	::LeaveCriticalSection(&lock_);
#else
	::pthread_spin_unlock(&lock_);
#endif
}
