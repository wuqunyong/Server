#ifndef __SHARED_UTILS_SINGLETON_H__
#define __SHARED_UTILS_SINGLETON_H__

#include <memory>

#include "SpinLock.h"

using std::auto_ptr;

template<typename Type, typename Lock = SpinLock>
class Singleton
{
protected:
	Singleton(void);
	~Singleton(void);

public:
	static Type* Instance(void);

protected:
	static auto_ptr<Type> instance_;
	static Lock lock_;
};

#include "Singleton.inl"

#endif
