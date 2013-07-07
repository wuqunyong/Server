#include "Opcodes.h"

#include <iostream>

Opcodes::Opcodes()
{

}

Opcodes::~Opcodes()
{
	for (std::set<IFacade*>::iterator ite = facades_.begin();
		ite != facades_.end();
		ite++)
	{
		delete *ite;
	}

	facades_.clear();
}

void Opcodes::Init()
{

}

void Opcodes::Update(void)
{
	for (std::set<IFacade*>::iterator ite = facades_.begin();
		ite != facades_.end();
		ite++)
	{
		(*ite)->Update();
	}
}

IDelegate* Opcodes::FindHandler(uint32_t opcodes)
{
	 std::map<uint32_t, IDelegate*>::iterator ite = handlers_.find(opcodes);
	 if (ite != handlers_.end())
	 {
		 return ite->second;
	 }
	 else
	 {
		 return NULL;
	 }
}

void Opcodes::Plug(IFacade* ptr_handler, std::map<uint32_t, IDelegate*> opcode_handler)
{
	this->facades_.insert(ptr_handler);

	for (std::map<uint32_t, IDelegate*>::iterator ite = opcode_handler.begin();
		ite != opcode_handler.end();
		ite++)
	{
		std::pair<std::map<uint32_t, IDelegate*>::iterator,bool> ret;
		ret = this->handlers_.insert(std::pair<uint32_t, IDelegate*>(ite->first, ite->second));
		if (ret.second == false)
		{
			std::cout << "opcopes: " << ite->first << " repeat " << std::endl;
		}
	}
}

//IDelegate* Opcodes::FindForwardHandler(uint32_t opcodes)
//{
//	std::map<uint32_t, IDelegate*>::iterator ite = forward_handlers_.find(opcodes);
//	if (ite != forward_handlers_.end())
//	{
//		return ite->second;
//	}
//	else
//	{
//		return NULL;
//	}
//}

//void Opcodes::ForwardPlug(IFacade* ptr_handler, std::map<uint32_t, IDelegate*> opcode_handler)
//{
//	this->facades_.insert(ptr_handler);
//
//	for (std::map<uint32_t, IDelegate*>::iterator ite = opcode_handler.begin();
//		ite != opcode_handler.end();
//		ite++)
//	{
//		std::pair<std::map<uint32_t, IDelegate*>::iterator,bool> ret;
//		ret = this->forward_handlers_.insert(std::pair<uint32_t, IDelegate*>(ite->first, ite->second));
//		if (ret.second == false)
//		{
//			std::cout << "opcopes: " << ite->first << " repeat " << __FILE__ << " " << __LINE__ << std::endl;
//		}
//	}
//}