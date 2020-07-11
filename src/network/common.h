#ifndef MOD_NETCOMMON
#define MOD_NETCOMMON

#include <stdexcept>

void guard(int value, const char* runtimeErrorMessage);

struct NetworkPacket {
  void* packet;
  unsigned int packetSize;
};

#endif