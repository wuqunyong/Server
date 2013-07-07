#ifndef __SHARED_SERIALIZATION_DELEGATE_H__
#define __SHARED_SERIALIZATION_DELEGATE_H__

#include "IDelegate.h"
#include "ByteBuffer.h"

template<typename Type>
class Delegate : public IDelegate
{
	typedef void (Type::*Function)(ByteBuffer* ptr);

public:
	Delegate(Type* object, Function function);
	virtual ~Delegate(void);

public:
	virtual void Invoke(ByteBuffer* ptr);

protected:
	Type* object_;
	Function function_;
};

#include "Delegate.inl"

#endif 
