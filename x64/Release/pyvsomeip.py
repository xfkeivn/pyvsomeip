import ctypes
from ctypes import *
from abc import ABC
from abc import abstractmethod

class MessageType(Structure):
    _fields_ = [("message", c_uint32),
                ("service", c_uint16),
                ("instance", c_uint16),
                ("method", c_uint16),
                ("payload_length", c_uint16),
                ("request_id", c_uint16),
                ("client_id", c_uint16),
                ("session_id", c_uint16),
                ("protocol_version", c_uint16),
                ("interface_version", c_uint16),
                ("message_type", c_uint8),
                ("return_code", c_uint8),
                ("is_available", c_bool),
                ("is_initial", c_bool),
                ("payload",c_ubyte*1000000)
                ]


class NativeCallback:
    def __init__(self, callback):
        self.callback = callback

    def __call__(self, service_id,instance_id,availablity):
        self.callback(service_id,instance_id,availablity)

class SomeIpClient(ABC):
    def __init__(self,application_name):
        b_string = application_name.encode('utf-8')
        self.application_name = ctypes.create_string_buffer(b_string)
        self.vsomeip_dll = ctypes.CDLL("pyvsomeip.dll")
        self.create_client = self.vsomeip_dll.create_client
        self.create_client.arg_types = [ctypes.c_char_p]
        self.create_client.restype = c_int
        self.send = self.vsomeip_dll.send
        self.send.arg_types = [c_int, c_uint16,c_uint16,c_uint16,POINTER(c_ubyte),c_uint]
        self.send.restype = c_int
        self.request_service = self.vsomeip_dll.request_service
        self.request_service.arg_types = [c_int, c_uint16,c_uint16]
        self.request_service.restype = c_int
        self.subscribe_event = self.vsomeip_dll.subscribe_event
        self.subscribe_event.arg_types = [c_int, c_uint16,c_uint16,c_uint16,c_uint16]
        self.subscribe_event.restype = c_int
        self.start = self.vsomeip_dll.start
        self.start.arg_types = [c_int]
        self.start.restype = c_int
        self.stop = self.vsomeip_dll.stop
        self.stop.arg_types = [c_int]
        self.stop.restype = c_int
        self.on_availability_cb_functype = CFUNCTYPE(None, c_uint16,c_uint16,c_bool)
        self.on_message_cb_functype = CFUNCTYPE(None, MessageType)
        self.on_state_cb_functype = CFUNCTYPE(None, c_uint8)
        self.client_id = None
        self.register_callbacks = self.vsomeip_dll.register_callbacks
        self.register_callbacks.arg_types = [c_int,self.on_availability_cb_functype,self.on_message_cb_functype, self.on_state_cb_functype]
        self.register_callbacks.restype = c_int
        self.on_availability_cb = self.on_availability_cb_functype(self.on_availability)
        self.on_message_cb = self.on_message_cb_functype(self.on_message)
        self.on_state_cb = self.on_state_cb_functype(self.on_state)
        self.register_callbacks(self.on_availability_cb,self.on_message_cb,self.on_state_cb)
        self.client_id = self.create_client(self.application_name)
        print (self.client_id)
        pass
    @abstractmethod
    def on_availability(self,service_id,instance_id,availablity):
        pass

    @abstractmethod
    def on_message(self,message):
        pass

    @abstractmethod
    def on_state(self,state):
        pass


    def someip_request_service(self,service_id,instance_id):
        return self.request_service(self.client_id,service_id,instance_id)

    def someip_subscribe_event(self, service_id, instance_id,event_group_id,event_id):
        return self.subscribe_event(self.client_id, service_id, instance_id,event_group_id,event_id)

    def someip_start(self):
        return self.start(self.client_id)

    def someip_stop(self):
        return self.stop(self.client_id)

    def someip_send(self,service_id,instance_id,method_id,payload):
        ARRAYTYPE = c_ubyte*len(payload)
        payload_array = ARRAYTYPE(*payload)
        return self.send(self.client_id,service_id,instance_id,method_id,payload_array,len(payload))


class MySomIPClient(SomeIpClient):

    def on_availability(self, service_id, instance_id, availablity):
        print (service_id,instance_id,availablity)

    def on_message(self, message):

        print(message)

    def on_state(self, state):
        print(state)

if __name__ == "__main__":
    app_name = "client-app"
    clientapp = MySomIPClient(app_name)
    clientapp.someip_start()
    pass;

