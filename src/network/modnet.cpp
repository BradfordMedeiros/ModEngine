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
 	size_t dataSize = sizeof(NetworkMessageResponse);
    NetworkMessageResponse networkErrorResponse{};
    networkErrorResponse.errorResponse = ResponseError{};
	sendData((char*)&networkErrorResponse, dataSize, closeSocket);
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
      	std::optional<NetworkMessageResponse> response = handleWireMessage(wireMessage);
   		size_t dataSize = sizeof(NetworkMessageResponse);
      	if (response.has_value()){
      		sendData((char*)&response.value(), dataSize, true);
      		return;
      	}
      }
      sendError(sendData, true);

    });
  }
}