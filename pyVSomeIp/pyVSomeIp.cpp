// pyVSomeIp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pyVSomeIp.hpp"


int convert_to_struct(const std::shared_ptr< message > &_response, SomeIpMessage &someipmessage)
{
	SomeIpMessage msg;
	msg.client_id = _response->get_client();
	msg.instance = _response->get_instance();
	msg.interface_version = _response->get_interface_version();
	msg.is_available = _response->is_reliable();
	msg.is_initial = _response->is_initial();
	msg.message = _response->get_message();
	msg.message_type = (MessageType)_response->get_message_type();
	msg.method = _response->get_method();
	if (_response->get_payload()->get_length() > MAX_SUPPORTED_PAYLOAD_SIZE)
		return ERROR_MESSAGE_PAYLOAD_OVER_SIZE;
	memcpy_s(msg.payload, _response->get_payload()->get_length(), _response->get_payload()->get_data(), _response->get_payload()->get_length());
	someipmessage = msg;
	return SUCCESS;

}

Client::Client(bool _use_tcp, bool _be_quiet, uint32_t _cycle,string applicationname)
		: app_(runtime::get()->create_application(application_name)),
		request_(runtime::get()->create_request(_use_tcp)),
		use_tcp_(_use_tcp),
		be_quiet_(_be_quiet),
		cycle_(_cycle),
		running_(true),
		blocked_(false),
		is_available_(false),
		sender_(std::bind(&Client::run, this))
		{
			application_name = applicationname;
			onAvailableCb_ = NULL;
			onMessageCb_ = NULL;
			onStateCb_ = NULL;
			init();
	}

	void Client::register_call_backs(on_availability_cb_py onAvailableCb, on_message_cb_py onMessageCb, on_state_cb_py onStateCb){
		this->onAvailableCb_ = onAvailableCb;
		this->onMessageCb_ = onMessageCb;
		this->onStateCb_ = onStateCb;
	}

	void Client::send(service_t service_id, instance_t instance_id, method_t method_id, std::vector< byte_t > its_payload_data)
	{
		request_->set_service(service_id);
		request_->set_instance(instance_id);
		request_->set_method(method_id);
		std::shared_ptr< payload > its_payload = runtime::get()->create_payload();
		for (std::size_t i = 0; i < 10; ++i) its_payload_data.push_back(i % 256);
		its_payload->set_data(its_payload_data);
		request_->set_payload(its_payload);
		send();
	
	}
	// must be called after the service is available and before send
	void Client::request_service(service_t service_id, instance_t instance_id)
	{
		app_->request_service(service_id, instance_id);
	}
	// must be called after the service is available for subscribe the event
	void Client::subscribe_event(service_t service_id, instance_t instance_id, eventgroup_t event_group_id, event_t event_id)
	{
		std::set<eventgroup_t> its_groups;
		its_groups.insert(event_group_id);
		app_->request_event(
			service_id,
			instance_id,
			event_id,
			its_groups,
			event_type_e::ET_FIELD);
		app_->subscribe(service_id, instance_id, event_group_id);
	}
	void Client::start() {
		app_->start();
	}

	void Client::stop() {
		app_->clear_all_handler();
		//app_->unsubscribe(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENTGROUP_ID);
		//app_->release_event(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID);
		//app_->release_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID);
		app_->stop();
	}



	bool Client::init() {
		if (!app_->init()) {
			std::cerr << "Couldn't initialize application" << std::endl;
			return false;
		}

		std::cout << "Client settings [protocol="
			<< (use_tcp_ ? "TCP" : "UDP")
			<< ":quiet="
			<< (be_quiet_ ? "true" : "false")
			<< ":cycle="
			<< cycle_
			<< "]"
			<< std::endl;

		app_->register_state_handler(
			std::bind(
				&Client::on_state,
				this,
				std::placeholders::_1));

		app_->register_message_handler(
			ANY_SERVICE, ANY_INSTANCE, ANY_METHOD,
			std::bind(&Client::on_message,
				this,
				std::placeholders::_1));

		app_->register_availability_handler(ANY_SERVICE, ANY_INSTANCE,
			std::bind(&Client::on_availability,
				this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	}

	void Client::on_state(state_type_e _state) {
		if (_state == state_type_e::ST_REGISTERED && this->onStateCb_!=NULL) {
			this->onStateCb_((uint8_t)_state);
		}
	}

	void Client::on_availability(service_t _service, instance_t _instance, bool _is_available) {
		std::cout << "Service ["
			<< std::setw(4) << std::setfill('0') << std::hex << _service << "." << _instance
			<< "] is "
			<< (_is_available ? "available." : "NOT available.")
			<< std::endl;
		if (this->onAvailableCb_ != NULL)
		{
			this->onAvailableCb_(_service, _instance, _is_available);
		}

	}

	void Client::on_message(const std::shared_ptr< message > &_response) {
		std::cout << "Received a response from Service ["
			<< std::setw(4) << std::setfill('0') << std::hex << _response->get_service()
			<< "."
			<< std::setw(4) << std::setfill('0') << std::hex << _response->get_instance()
			<< "] to Client/Session ["
			<< std::setw(4) << std::setfill('0') << std::hex << _response->get_client()
			<< "/"
			<< std::setw(4) << std::setfill('0') << std::hex << _response->get_session()
			<< "]"
			<< std::endl;
		///if (is_available_ )
		///	send();
		if (onMessageCb_!=NULL)
		{
			SomeIpMessage message;
			if (convert_to_struct(_response, message) == SUCCESS)
				onMessageCb_(message);
		}
	}

	void Client::send() {
		if (!be_quiet_)
		{
			std::lock_guard< std::mutex > its_lock(mutex_);
			blocked_ = true;
			condition_.notify_one();
		}
	}

	void Client::run() {
		while (running_) {
			{
				std::unique_lock<std::mutex> its_lock(mutex_);
				while (!blocked_) condition_.wait(its_lock);
				if (is_available_) {
					app_->send(request_);
					std::cout << "Client/Session ["
						<< std::setw(4) << std::setfill('0') << std::hex << request_->get_client()
						<< "/"
						<< std::setw(4) << std::setfill('0') << std::hex << request_->get_session()
						<< "] sent a request to Service ["
						<< std::setw(4) << std::setfill('0') << std::hex << request_->get_service()
						<< "."
						<< std::setw(4) << std::setfill('0') << std::hex << request_->get_instance()
						<< "]"
						<< std::endl;
					blocked_ = false;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(cycle_));
		}
	}



