#ifndef MOD_HTTP
#define MOD_HTTP

#include <string>
#include <cstring>
#include <unordered_map>
#include <curl/curl.h>
#include "../common/util.h"

bool downloadFile(std::string url, std::string outputFile);
std::optional<std::string> downloadFileInMemory(std::string urlStr, bool* isSuccess);
bool isServerOnline(std::string url);

// More utility for specific config
struct ServerConfigProperties {
  std::vector<Token> properties;
};
std::optional<ServerConfigProperties> getServerConfig(std::string urlStr,  bool* isSuccess);
std::optional<std::string> getProperty(ServerConfigProperties& config, std::string target, std::string attribute);

#endif
