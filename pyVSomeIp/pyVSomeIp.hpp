// pyVSomeIp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma once
#include <iostream>
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <vsomeip/vsomeip.hpp>
#include "pyvsomeip.h"
using namespace std;
using namespace vsomeip;


typedef void(*on_availability_cb_py)(uint16_t _service, uint16_t _instance, bool _is_available);
typedef void(*on_message_cb_py)(SomeIpMessage response);
typedef void(*on_state_cb_py)(uint8_t _state);

typedef void(*on_availability_cb)(service_t _service, instance_t _instance, bool _is_available);
typedef void(*on_message_cb)(const shared_ptr< message > &_response);
typedef void(*on_state_cb)(state_type_e _state);

class Client {

public:
	Client(bool _use_tcp, bool _be_quiet, uint32_t _cycle, string application_name = "");
	void register_call_backs(on_availability_cb_py onAvailableCb, on_message_cb_py onMessageCb, on_state_cb_py onStateCb);
	void send(service_t service_id, instance_t instance_id, method_t method_id, std::vector< byte_t > payload);
	void request_service(service_t service_id, instance_t instance_id);
	// must be called after the service is available for subscribe the event
	void subscribe_event(service_t service_id, instance_t instance_id, eventgroup_t event_group_id, event_t event_id);
	void start();
	void stop();
	string get_application_name() { return application_name; }


private:
	bool init();
	void on_state(state_type_e _state);
	void on_availability(service_t _service, instance_t _instance, bool _is_available);
	void on_message(const std::shared_ptr< message > &_response);
	void send();
	void run();

private:
	std::shared_ptr< application > app_;
	std::shared_ptr< message > request_;
	std::string application_name;
	bool use_tcp_;
	bool be_quiet_;
	uint32_t cycle_;
	session_t session_;
	std::mutex mutex_;
	std::condition_variable condition_;
	bool running_;
	bool blocked_;
	bool is_available_;
	//std::vector<vsomeip_v3::eventgroup_t> requested_event_groups;
	//std::vector<vsomeip_v3::eventgroup_t> requested_services;
	std::thread sender_;
	on_availability_cb_py onAvailableCb_;
	on_message_cb_py onMessageCb_;
	on_state_cb_py onStateCb_;
};





