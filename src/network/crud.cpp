#include "./crud.h"

std::optional<NetworkMessageResponse> handleTestRequest(NetworkMessage& networkMessage, bool* error){
	TestApiRequest& testMessage = networkMessage.testRequest;
	TestApiResponse testApiResponse {
		.responseValue = 745,
	};
	NetworkMessageResponse networkMessageResponse;
	networkMessageResponse.testApiResponse = testApiResponse;
	return networkMessageResponse;
}
WireMessage createWireMessage(TestApiRequest& testRequest){
	return WireMessage {
		.version = 0,
		.apiType = typeid(TestApiRequest).hash_code(),
		.networkMessage = {
			.testRequest = testRequest,
		}
	};
}

std::optional<NetworkMessageResponse> handleAnotherApiRequest(NetworkMessage& networkMessage, bool* error){
	AnotherApiRequest& apiRequest = networkMessage.anotherRequest;

	std::cout << "handleAnotherApiRequest not yet implemented" << std::endl;
	assert(false);
	return std::nullopt;
}

WireMessage createWireMessage(AnotherApiRequest& anotherRequest){
	return WireMessage {
		.version = 0,
		.apiType = typeid(AnotherApiRequest).hash_code(),
		.networkMessage = {
			.anotherRequest = anotherRequest,
		}
	};
}


std::unordered_map<std::size_t, std::function<std::optional<NetworkMessageResponse>(NetworkMessage&, bool* error)>> requestTypeToFn {
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

WireMessageResponse handleWireMessage(WireMessage& wireMessage){
	WireMessageResponse wireResponse{};
	std::memset(&wireResponse, 0, sizeof(wireResponse));
	wireResponse.version = 0;
	wireResponse.error = false;

	auto apiType = wireMessage.apiType;
	if (requestTypeToFn.find(apiType) == requestTypeToFn.end()){
		std::cout << "crud: invalid api type: " << apiType << std::endl;
		wireResponse.error = true;
		return wireResponse;
	}

	auto response = requestTypeToFn.at(apiType)(wireMessage.networkMessage, &wireResponse.error);
	if(response.has_value()){
		wireResponse.response = response.value();
	}else{
		wireResponse.error = true;
	}
	return wireResponse;
}

