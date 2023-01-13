#include "pyvsomeip.h"
#include "pyVSomeIp.hpp"
#include <map>
using namespace std;

map<int,Client*> Clients;
int Max_Client_Id = 0;
int create_client(char * application_name)
{
	map<int, Client*>::iterator itor = Clients.begin();
	while (itor != Clients.end())
	{
		int key = (*itor).first;
		Client* c = (*itor).second;
		string appname = c->get_application_name();
		if (appname == application_name)
		{
			return ERROR_APPLICATION_NAME_EXIST;
		}
		itor++;
	}
	

	Client *client = new Client(true, false, 10, string(application_name));
	Clients[Max_Client_Id] = client;
	Max_Client_Id++;
	//printf(application_name);
	return Max_Client_Id-1;
}
__declspec(dllexport) int register_callbacks(int client_id, on_availability_cb_py cb1, on_message_cb_py cb2, on_state_cb_py cb3)
{
	if (Clients.find(client_id) != Clients.end())
	{
		Clients.find(client_id)->second->register_call_backs(cb1, cb2, cb3);
	}
	return ERROR_APPLICATION_CLIENT_NOT_EXIST;
}
__declspec(dllexport) int send(int client_id, uint16_t service_id, uint16_t instance_id, uint16_t method_id, unsigned char * payloadbuffer, unsigned int payload_length)
{

	if (Clients.find(client_id) != Clients.end())
	{
		std::vector< byte_t > payload;
		for (int i = 0; i < payload_length; i++)
			payload.push_back(payloadbuffer[i]);
		Clients.find(client_id)->second->send(service_id, instance_id, method_id,payload);
		return SUCCESS;
	}
	return ERROR_APPLICATION_CLIENT_NOT_EXIST;
}
// must be called after the service is available for subscribe the event
__declspec(dllexport) int request_service(int client_id, uint16_t service_id, uint16_t instance_id)
{
	if (Clients.find(client_id) != Clients.end())
	{
		Clients.find(client_id)->second->request_service(service_id, instance_id);
		return SUCCESS;
	}
	return ERROR_APPLICATION_CLIENT_NOT_EXIST;
}
__declspec(dllexport) int subscribe_event(int client_id, uint16_t service_id, uint16_t instance_id, uint16_t event_group_id, uint16_t event_id)
{
	if (Clients.find(client_id) != Clients.end())
	{
		Clients.find(client_id)->second->subscribe_event(service_id, instance_id, event_group_id, event_id);
		return SUCCESS;
	}
	return ERROR_APPLICATION_CLIENT_NOT_EXIST;
}
__declspec(dllexport) int start(int client_id)
{
	if (Clients.find(client_id) != Clients.end())
	{
		Clients.find(client_id)->second->start();
		return SUCCESS;
	}
	return ERROR_APPLICATION_CLIENT_NOT_EXIST;
}
__declspec(dllexport) int stop(int client_id)
{
	if (Clients.find(client_id) != Clients.end())
	{
		Clients.find(client_id)->second->stop();
		return SUCCESS;
	}
	return ERROR_APPLICATION_CLIENT_NOT_EXIST;
}