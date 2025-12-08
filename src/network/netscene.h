#ifndef MOD_NETSCENE
#define MOD_NETSCENE

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h> 
#include <string>
#include <functional>
#include <iostream>
#include <functional>
#include <cstring>
#include <errno.h>
#include <assert.h>
#include <optional>

void guard(int value, const char* runtimeErrorMessage);

struct ConnectionInfo {
  short unsigned int port;
  int socketFd;
  std::string ipAddress;
};

// Server
struct modsocket {
  int socketFd;
  fd_set fds;
  int maxFd;
  std::vector<ConnectionInfo> infos;
};
      
struct tcpServer {
  modsocket server;
};

struct ClientConnection {
  bool isConnected = false;
  int currentSocket = -1;
};
struct NetCode {
  tcpServer tServer;
};

NetCode initNetCode(bool bootstrapperMode, std::function<std::string(std::string)> readFile);
void onNetCode(NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode);

// client 
void connectServer(std::string data);
void disconnectServer();

std::string sendMessage(std::string dataToSend);
std::vector<uint8_t> sendMessage(const char* data, size_t size);
std::vector<uint8_t> sendMessageAsBytes(const char* data, size_t size);

template <typename T> 
std::optional<T> convertToType(std::vector<uint8_t>& value){
  T msg;

  bool isExpectedSize = value.size() == sizeof(T);
  if (!isExpectedSize){
    std::cout << "netscene: unexpected size: expected = " << sizeof(T) << ", got = " << value.size() << ", value = "  << std::string(reinterpret_cast<const char*>(value.data()), value.size()) << std::endl;
    return std::nullopt;
  }
  //modassert(, std::string("unexpected buffer size: expected = ") + std::to_string(sizeof(T)) + ", actual = " + std::to_string(response.size())); 
  std::memcpy(&msg, value.data(), sizeof(msg)); 
  return msg;
}

template <typename T>
std::optional<T> sendMessage(const char* data, size_t size){
  auto values = sendMessageAsBytes(data, size);
  return convertToType<T>(values);
}

template <typename ReturnType, typename TypeToSend> 
std::optional<ReturnType> sendMessageAnyType(TypeToSend& dataToSend){
  return sendMessage<ReturnType>((char*)&dataToSend, sizeof(TypeToSend));
}


struct MessageToSend {
  int value = 123;
};


#endif