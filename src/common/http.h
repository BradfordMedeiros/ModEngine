#ifndef MOD_HTTP
#define MOD_HTTP

#include <string>
#include <unordered_map>
#include <curl/curl.h>
#include "./util.h"

bool downloadFile(std::string url, std::string outputFile);
bool isServerOnline(std::string url);

#endif
