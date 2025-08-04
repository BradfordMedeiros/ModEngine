#ifndef MOD_HTTP
#define MOD_HTTP

#include <string>
#include <cstring>
#include <unordered_map>
#include <curl/curl.h>
#include "./util.h"

bool downloadFile(std::string url, std::string outputFile);
std::optional<std::string> downloadFileInMemory(std::string urlStr, bool* isSuccess);

bool isServerOnline(std::string url);

#endif
