#include "./modnet.h"

extern ClientConnection client;

NetCode initNetCode(bool bootstrapperMode, std::function<std::string(std::string)> readFile){
  std::cout << "INFO: running in server bootstrapper mode" << std::endl;
  NetCode netcode {};
  if (bootstrapperMode){
    netcode.tServer = initTcpServer(readFile);
  }
  return netcode;
}


void sendError(std::function<void(char* data, size_t size, bool closeSocket)> sendData, bool closeSocket){
 	size_t dataSize = sizeof(WireMessageResponse);
  	WireMessageResponse wireResponse{};
	std::memset(&wireResponse, 0, sizeof(wireResponse));
	wireResponse.version = 0;
	wireResponse.error = true;

	sendData((char*)&wireResponse, dataSize, closeSocket);
}

void onNetCode(NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode){
  maybeReadClientMessage(client, onClientMessage);
  if (bootstrapperMode){
    tcpServer& tserver = netcode.tServer;
    getDataFromServerSocket(tserver.server, [&tserver](char* data, size_t size, int socketFd, std::function<void(char* data, size_t size, bool closeSocket)> sendData) -> void {      
      bool expectedSize = sizeof(WireMessage) == size;
      if (expectedSize){
      	WireMessage wireMessage{};
      	std::memcpy(&wireMessage, data,size); 
      	std::cout << "modnet wireMessage: version = " << wireMessage.version << ", apiType = " << wireMessage.apiType << std::endl;

      	WireMessageResponse response = handleWireMessage(wireMessage);
      	sendData((char*)&response, sizeof(WireMessageResponse), true);
      	return;
      }
      sendError(sendData, true);

    });
  }
}

std::optional<WireMessageResponse> sendWireMessage(WireMessage wireMessage){
   auto wireNetworkResponse = sendMessageAnyType<WireMessageResponse, WireMessage>(wireMessage);
   if (!wireNetworkResponse.has_value()){
        std::cout << "modnet did not get expected type back" << std::endl;
        return std::nullopt;
   }
   return wireNetworkResponse;
}