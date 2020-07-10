#include "./common.h"

void guard(int value, const char* runtimeErrorMessage){
  if (value < 0){
    throw std::runtime_error(runtimeErrorMessage);
  }
}

NetworkPacket toNetworkPacket(UdpPacket& packet){
  NetworkPacket netpacket {
    .packet = &packet,
    .packetSize = sizeof(packet),
  };
  return netpacket;
}