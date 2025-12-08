#include "./crud.h"


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