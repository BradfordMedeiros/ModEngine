#ifndef MOD_CRUD
#define MOD_CRUD

#include <typeinfo>
#include <string>
#include <queue>
#include <optional>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <assert.h>
#include <cstring>

struct TestApiRequest {
	int value;
};

struct AnotherApiRequest {
	int valueOne;
	int valueTwo;
};
union NetworkMessage {
	TestApiRequest testRequest;
	AnotherApiRequest anotherRequest;
};
struct WireMessage {
	int version;
	std::size_t apiType;
	NetworkMessage networkMessage;
};

struct ResponseVoid{};
struct TestApiResponse{
	int responseValue;
};
union NetworkMessageResponse {
	TestApiResponse testApiResponse;
	ResponseVoid anotherApiResponse;
};

struct WireMessageResponse {
	int version;
	bool error;
	NetworkMessageResponse response;
};

WireMessageResponse handleWireMessage(WireMessage& wireMessage);

WireMessage createWireMessage(TestApiRequest& testRequest);
WireMessage createWireMessage(AnotherApiRequest& anotherRequest);


#endif 
