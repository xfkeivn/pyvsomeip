#include <stdint.h>


#ifndef PYVSOMEIP
#define PYVSOMEIP

#define MAX_SUPPORTED_PAYLOAD_SIZE 1000000
#define SUCCESS 0
#define ERROR_APPLICATION_NAME_EXIST -1
#define ERROR_APPLICATION_CLIENT_NOT_EXIST -2
#define ERROR_MESSAGE_PAYLOAD_OVER_SIZE -3

extern "C" {



typedef enum  {
	MT_REQUEST = 0x00,
	MT_REQUEST_NO_RETURN = 0x01,
	MT_NOTIFICATION = 0x02,
	MT_REQUEST_ACK = 0x40,
	MT_REQUEST_NO_RETURN_ACK = 0x41,
	MT_NOTIFICATION_ACK = 0x42,
	MT_RESPONSE = 0x80,
	MT_ERROR = 0x81,
	MT_RESPONSE_ACK = 0xC0,
	MT_ERROR_ACK = 0xC1,
	MT_UNKNOWN = 0xFF
} MessageType;


typedef enum  {
	E_OK = 0x00,
	E_NOT_OK = 0x01,
	E_UNKNOWN_SERVICE = 0x02,
	E_UNKNOWN_METHOD = 0x03,
	E_NOT_READY = 0x04,
	E_NOT_REACHABLE = 0x05,
	E_TIMEOUT = 0x06,
	E_WRONG_PROTOCOL_VERSION = 0x07,
	E_WRONG_INTERFACE_VERSION = 0x08,
	E_MALFORMED_MESSAGE = 0x09,
	E_WRONG_MESSAGE_TYPE = 0x0A,
	E_UNKNOWN = 0xFF
}ReturnCode;

typedef enum  {
	ST_REGISTERED = 0x0,
	ST_DEREGISTERED = 0x1
}StateType;

typedef struct _Message {
	uint32_t message;
	uint16_t service;
	uint16_t instance;
	uint16_t method;
	uint32_t payload_length;
	uint32_t request_id;
	uint16_t client_id;
	uint16_t session_id;
	uint8_t protocol_version;
	uint8_t interface_version;
	MessageType message_type;
	ReturnCode  return_code;
	bool is_available;
	bool is_initial;
	unsigned char payload[MAX_SUPPORTED_PAYLOAD_SIZE];
} SomeIpMessage;


typedef void(*on_availability_cb_py)(uint16_t _service, uint16_t _instance, bool _is_available);
typedef void(*on_message_cb_py)(SomeIpMessage response);
typedef void(*on_state_cb_py)(uint8_t _state);

__declspec(dllexport) int create_client(char * application_name);
__declspec(dllexport) int register_callbacks(int client_id, on_availability_cb_py cb1, on_message_cb_py cb2, on_state_cb_py cb3);
__declspec(dllexport) int send(int client_id, uint16_t service_id, uint16_t instance_id, uint16_t method_id, unsigned char * payloadbuffer, unsigned int payload_length);
// must be called after the service is available for subscribe the event
__declspec(dllexport) int request_service(int client_id, uint16_t service_id, uint16_t instance_id);
__declspec(dllexport) int subscribe_event(int client_id, uint16_t service_id, uint16_t instance_id, uint16_t event_group_id, uint16_t event_id);
__declspec(dllexport) int start(int client_id);
__declspec(dllexport) int stop(int client_id);

}




#endif


