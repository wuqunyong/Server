#include "Singleton.h"
#include "Guard.h"
#include "Lock.h"

template<typename Type, typename Lock> 
auto_ptr<Type> Singleton<Type, Lock>::instance_;

template<typename Type, typename Lock> 
Lock Singleton<Type, Lock>::lock_;

template<typename Type, typename Lock>
Singleton<Type, Lock>::Singleton(void)
{
}

template<typename Type, typename Lock>
Singleton<Type, Lock>::~Singleton(void)
{
}

template<typename Type, typename Lock> 
Type* Singleton<Type, Lock>::Instance(void)
{
    if (NULL == instance_.get())
    {
        Guard guard(lock_);
        if (NULL == instance_.get())
		{
			instance_.reset(::new Type());
		}
    }

    return instance_.get();
}
