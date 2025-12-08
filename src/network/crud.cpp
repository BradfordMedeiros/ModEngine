#include "./crud.h"


struct TestApiRequest {
	int value;
};
struct TestApiResponse{};

struct AnotherApiRequest {
	int valueOne;
	int valueTwo;
};

union NetworkMessage {
	TestApiRequest testRequest;
	AnotherApiRequest anotherRequest;
};

struct ResponseVoid{};
union NetworkMessageResponse {
	ResponseVoid voidResponse;
	TestApiResponse testApiResponse;
};

std::optional<NetworkMessageResponse> handleTestRequest(NetworkMessage& networkMessage){
	TestApiRequest& testMessage = networkMessage.testRequest;

	return std::nullopt;
}
std::optional<NetworkMessageResponse> handleAnotherApiRequest(NetworkMessage& networkMessage){
	AnotherApiRequest& apiRequest = networkMessage.anotherRequest;
	return std::nullopt;
}

std::unordered_map<std::size_t, std::function<std::optional<NetworkMessageResponse>(NetworkMessage&)>> requestTypeToFn {
	{ typeid(TestApiRequest).hash_code(), handleTestRequest },
	{ typeid(AnotherApiRequest).hash_code(), handleAnotherApiRequest },
};


struct WireMessage {
	int version;
	std::size_t apiType;
	NetworkMessage networkMessage;
};

WireMessage createWireMessage(NetworkMessage networkMessage, std::size_t apiType){
	return WireMessage {
		.version = 0,
		.apiType = apiType, 
		.networkMessage = networkMessage,
	};
}

std::optional<NetworkMessageResponse> handleWireMessage(WireMessage& wireMessage){
	auto apiType = wireMessage.apiType;
	if (requestTypeToFn.find(apiType) == requestTypeToFn.end()){
		std::cout << "crud: invalid api type: " << apiType << std::endl;
		return std::nullopt;
	}

	auto response = requestTypeToFn.at(apiType)(wireMessage.networkMessage);
	if(!response.has_value()){
		NetworkMessageResponse response{};
		response.voidResponse = ResponseVoid{};
		return response;
	}
	return response;
}