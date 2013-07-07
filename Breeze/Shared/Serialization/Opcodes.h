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
		CREATE_SESSION,		    //�������ӽڵ�
		CLOSE_SESSION,          //�������ӽڵ� 
		ROUTING,                //GATEWAY_SERVER:ת�����ݰ�·��Э��
		REQUEST_TEST,           //ROLE_SERVER:�ͻ����������ݰ�
		RESPONSE_TEST,          //ROLE_SERVER:�ظ��ͻ����������ݰ�
		NOTIFY_TEST,            //ROLE_SERVER:֪ͨ�ͻ������ݰ�
		INTERNAL_REQUEST_TEST,  //�������ڲ��������ݰ�
		INTERNAL_RESPONSE_TEST, //�������ڲ��ظ��������ݰ�
		INTERNAL_NOTIFY_TEST,   //�������ڲ�֪ͨ���ݰ�
		CLIENT_LINKING_GATEWAY, //�ͻ��˵�ǰ��������Ǹ��������� 
		REGISTER_GATEWAY_ADDRESS, //��LOGIN_SERVERע��GATEWAY��ַ
		NOTIFY_GATEWAY_LINKING_SERVICE, //���ڲ��ڵ�֪ͨ�������ӵ��ڲ�����

		L_PLAYER_BINDING_GATEWAY_REQUEST,//��½���������������
		G_PLAYER_BINDING_GATEWAY_RESPONSE,//��½����������ػظ�

		S_REQUEST_ERROR_RESPONSE,   //�ͻ����������ظ�

		PLATFORM_ACCOUNT_LOGIN_REQUEST,  //ƽ̨�˺ŵ�½����
		PLATFORM_ACCOUNT_LOGIN_RESPONSE, //ƽ̨�˺ŵ�½�ظ�

		C_GET_ROLE_REQUEST, //��ý�ɫ����
		S_GET_ROLE_RESPONSE,//��ý�ɫ�ظ�

		S_GATEWAY_ADDRESS_NOTIFY, //���ص�ַ֪ͨ

		
	};
	Opcodes();
	~Opcodes();

	void Init();
	void Update(void);

	IDelegate* FindHandler(uint32_t opcodes);
	void Plug(IFacade* ptr_handler, std::map<uint32_t, IDelegate*> opcode_handler);

	//����·��ת��
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
