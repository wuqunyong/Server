#ifndef __SHARED_SERIALIZATION_OPCODES_H__
#define __SHARED_SERIALIZATION_OPCODES_H__

#include <map>
#include <vector>
#include <set>

#include "IDelegate.h"
#include "Delegate.h"
#include "ByteBuffer.h"

class Opcodes
{
public:
	enum
	{
		ECHO  = 0,
		CREATE_SESSION,		    //创建连接节点
		CLOSE_SESSION,          //销毁连接节点 
		ROUTING,                //GATEWAY_SERVER:转发数据包路由协议
		REQUEST_TEST,           //ROLE_SERVER:客户端请求数据包
		RESPONSE_TEST,          //ROLE_SERVER:回复客户端请求数据包
		NOTIFY_TEST,            //ROLE_SERVER:通知客户端数据包
		INTERNAL_REQUEST_TEST,  //服务器内部请求数据包
		INTERNAL_RESPONSE_TEST, //服务器内部回复请求数据包
		INTERNAL_NOTIFY_TEST,   //服务器内部通知数据包
		CLIENT_LINKING_GATEWAY, //客户端当前或最近于那个网关连接 
		REGISTER_GATEWAY_ADDRESS, //向LOGIN_SERVER注册GATEWAY地址
		NOTIFY_GATEWAY_LINKING_SERVICE, //先内部节点通知网关连接的内部服务

		L_PLAYER_BINDING_GATEWAY_REQUEST,//登陆服绑定玩家网关请求
		G_PLAYER_BINDING_GATEWAY_RESPONSE,//登陆服绑定玩家网关回复

		S_REQUEST_ERROR_RESPONSE,   //客户端请求错误回复

		PLATFORM_ACCOUNT_LOGIN_REQUEST,  //平台账号登陆请求
		PLATFORM_ACCOUNT_LOGIN_RESPONSE, //平台账号登陆回复

		C_GET_ROLE_REQUEST, //获得角色请求
		S_GET_ROLE_RESPONSE,//获得角色回复

		S_GATEWAY_ADDRESS_NOTIFY, //网关地址通知

		
	};
	Opcodes();
	~Opcodes();

	void Init();
	void Update(void);

	IDelegate* FindHandler(uint32_t opcodes);
	void Plug(IFacade* ptr_handler, std::map<uint32_t, IDelegate*> opcode_handler);

	//负责路由转发
	//IDelegate* FindForwardHandler(uint32_t opcodes);
	//void ForwardPlug(IFacade* ptr_handler, std::map<uint32_t, IDelegate*> opcode_handler);


private:
	 std::map<uint32_t, IDelegate*> handlers_;
	 std::set<IFacade*> facades_;

	 //std::map<uint32_t, IDelegate*> forward_handlers_;
};

template<typename Type> 
void RegisterOpcodes(std::map<uint32_t, IDelegate*>& container, uint32_t opcode, Type* object, void (Type::*function)(ByteBuffer*))
{
	IDelegate* deleg  = new Delegate<Type>(object, function);
	container.insert(std::pair<uint32_t, IDelegate*>(opcode, deleg));
}

#endif 
