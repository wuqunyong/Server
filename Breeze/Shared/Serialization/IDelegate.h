
#ifndef __SHARED_SERIALIZATION_IDELEGATE_H__
#define __SHARED_SERIALIZATION_IDELEGATE_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>

#include "ByteBuffer.h"

class IDelegate
{
public:
	virtual void Invoke(ByteBuffer* ptr) = 0;

public:
	virtual ~IDelegate(void) { }
};

class IFacade
{
public:
	IFacade(void){}
	virtual ~IFacade(void){}

public:
	virtual void Initialize(void) = 0;
	virtual void Update(void) = 0;
	virtual std::map<uint32_t, IDelegate*> GenOpcodes(void) = 0;
	//virtual std::map<uint32_t, IDelegate*> GenForwardOpcodes(void) = 0;
};

#endif
