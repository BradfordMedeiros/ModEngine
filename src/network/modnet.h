#ifndef MOD_MODNET
#define MOD_MODNET

#include "./netscene.h"
#include "./crud.h"

struct NetCode {
  tcpServer tServer;
};

NetCode initNetCode(bool bootstrapperMode, std::function<std::string(std::string)> readFile);
void onNetCode(NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode);

#endif