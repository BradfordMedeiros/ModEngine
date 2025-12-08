#ifndef MOD_CRUD
#define MOD_CRUD

#include <typeinfo>
#include <string>
#include <queue>
#include <optional>
#include <unordered_map>
#include <functional>
#include <iostream>

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

struct ResponseError{};
struct ResponseVoid{};
struct TestApiResponse{};
union NetworkMessageResponse {
	ResponseError errorResponse;
	ResponseVoid voidResponse;
	TestApiResponse testApiResponse;
};

std::optional<NetworkMessageResponse> handleWireMessage(WireMessage& wireMessage);

#endif 
