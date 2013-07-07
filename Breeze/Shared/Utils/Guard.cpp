#include "Guard.h"
#include "Lock.h"

Guard::Guard(Lock& lock) :
	lock_(lock)
{
	lock_.Acquire();
}

Guard::~Guard(void)
{
	lock_.Release();
}

