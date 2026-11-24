#ifndef MOD_HTTP_SERVER
#define MOD_HTTP_SERVER

#include <functional>
#include <string>
#include <unordered_map>

struct HttpRequest {
  std::string method;
  std::string path;
  std::string body;
  std::unordered_map<std::string, std::string> headers;
  std::unordered_map<std::string, std::string> query;
};

struct HttpResponse {
  int status = 200;
  std::string contentType = "text/plain";
  std::string body;
  std::unordered_map<std::string, std::string> headers;
};

using HttpHandler = std::function<void(const HttpRequest&, HttpResponse&)>;

void httpGet(const std::string& path, HttpHandler handler);
void httpStatic(const std::string& urlPrefix, const std::string& root);
bool httpServerStart(const std::string& endpoint);
void httpServerUpdate();
void httpServerStop();

void httpResponseSend(HttpResponse& response, int status, const std::string& contentType, const std::string& body);
void httpResponseJson(HttpResponse& response, int status, const std::string& body);

#endif
