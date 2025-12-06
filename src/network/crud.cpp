#include "./crud.h"


struct TestApiRequest {
	int value;
};

struct AnotherApiRequest {
	int valueOne;
	int valueTwo;
};


enum RequestHandlerType {
	TestRequest = 1,
};

void handleTestRequest(void* data){

}
void handleAnotherApiRequest(void* data){

}

union NetworkMessage {
	TestApiRequest testRequest;
	AnotherApiRequest anotherRequest;
};

std::unordered_map<std::size_t, std::function<void(void*)>> requestTypeToFn {
	{ typeid(TestApiRequest).hash_code(), handleTestRequest },
	{ typeid(AnotherApiRequest).hash_code(), handleAnotherApiRequest },
};


struct WireMessage {
	int version;
	std::size_t apiType;
	NetworkMessage networkMessage;
	std::vector<std::string> values;
};
struct SimulatedNetwork {
	std::queue<WireMessage> networkBuffer;

};
SimulatedNetwork simulatedNetwork;

std::optional<NetworkMessage> receiveData(){
	return std::nullopt;
}
void sendData(NetworkMessage& networkMessage, std::size_t apiType){
	simulatedNetwork.networkBuffer.push(WireMessage {
		.version = 0,
		.apiType = apiType,
		.networkMessage = networkMessage,
	});
}


void sendToTestApi(){
	TestApiRequest value {
		.value = 10,
	};
	NetworkMessage networkMessage{};
	networkMessage.testRequest = value;
	sendData(networkMessage, typeid(TestApiRequest).hash_code());
}

void handleRequest(void* request){
}


/////////////////

struct tcpSocket {
	int socketFd;
	fd_set fds;
	int maxFd;
};
