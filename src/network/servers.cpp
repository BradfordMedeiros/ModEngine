#include "./servers.h"

std::map<std::string, std::string> parseConfigData(std::string serverConfig){
  std::map<std::string, std::string> serverNameToIp;
  auto serverInfos = filterWhitespace(split(serverConfig, '\n'));

  for (auto serverInfo : serverInfos){
    auto infos = filterWhitespace(split(serverInfo, ' '));
    assert(infos.size() == 2);
    auto serverName = infos.at(0);
    auto serverIp = infos.at(1);
    serverNameToIp[serverName] = serverIp;      // validate server ip at some point
  }
  return serverNameToIp;
}

ServerBrowser loadServerInfo(){
  ServerBrowser browser {
    .serverNameToIp = parseConfigData(loadFile("./res/data/servers")),
  };
  return browser;
}

std::string handleListServer(ServerBrowser& browser){
  std::string servers = "";
  for (auto [serverName, serverIp] : browser.serverNameToIp){
    servers = servers + serverName + " " + serverIp + "\n";
  }
  return servers;
}

void launchServer(std::function<void(std::string)> onTcpData){
  auto browser = loadServerInfo();
  
  std::cout << "INFO: create server start" << std::endl;
  auto server = createServer();
  std::cout << "INFO: create server end" << std::endl;

  while(true){
    getDataFromSocket(server, [&browser, onTcpData](std::string request) -> std::string {      // @TODO probably use byte encoding for this instead of using string style comparisons
      auto requestLines = split(request, '\n');
      auto requestHeader = requestLines.at(0);
      std::cout << "request is: " << request << std::endl;

      std::string response = "ok";
      if (requestHeader == "list-servers"){
        response = handleListServer(browser);
      } else if (requestHeader == "connect"){
        response = "ack";
      }else if (requestHeader == "type:data"){
        auto data = request.substr(10);
        onTcpData(data);
        response = "ok";
      }

      std::cout << "response is: " << std::endl << response << std::endl;
      return response;
    });
  }
}