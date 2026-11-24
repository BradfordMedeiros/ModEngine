#include "./http_server.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace {

struct Route {
  std::string method;
  std::string path;
  HttpHandler handler;
};

struct StaticRoute {
  std::string prefix;
  std::filesystem::path root;
};

struct Client {
  int fd;
  std::string data;
};

int serverSocket = -1;
std::vector<Client> clients;
std::vector<Route> routes;
std::vector<StaticRoute> staticRoutes;

bool setNonBlocking(int fd) {
  auto flags = fcntl(fd, F_GETFL, 0);
  return flags >= 0 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

std::string statusText(int status) {
  switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 409: return "Conflict";
    case 500: return "Internal Server Error";
    default: return "Response";
  }
}

std::string contentType(const std::filesystem::path& path) {
  auto extension = path.extension().string();
  if (extension == ".html" || extension == ".htm") return "text/html";
  if (extension == ".css") return "text/css";
  if (extension == ".js") return "application/javascript";
  if (extension == ".json") return "application/json";
  if (extension == ".png") return "image/png";
  if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
  if (extension == ".svg") return "image/svg+xml";
  if (extension == ".txt") return "text/plain";
  return "application/octet-stream";
}

std::string urlDecode(const std::string& value) {
  std::string decoded;
  for (size_t i = 0; i < value.size(); i++) {
    if (value[i] == '%' && i + 2 < value.size()) {
      char hex[3] = {value[i + 1], value[i + 2], '\0'};
      char* end = nullptr;
      auto number = std::strtol(hex, &end, 16);
      if (*end == '\0') {
        decoded.push_back(static_cast<char>(number));
        i += 2;
        continue;
      }
    }
    decoded.push_back(value[i] == '+' ? ' ' : value[i]);
  }
  return decoded;
}

bool parseRequest(const std::string& raw, HttpRequest& request) {
  auto headerEnd = raw.find("\r\n\r\n");
  if (headerEnd == std::string::npos) return false;

  std::istringstream stream(raw.substr(0, headerEnd));
  std::string requestLine;
  if (!std::getline(stream, requestLine)) return false;
  if (!requestLine.empty() && requestLine.back() == '\r') requestLine.pop_back();

  std::istringstream requestLineStream(requestLine);
  std::string target;
  std::string version;
  if (!(requestLineStream >> request.method >> target >> version)) return false;
  if (version != "HTTP/1.0" && version != "HTTP/1.1") return false;

  auto queryStart = target.find('?');
  auto path = queryStart == std::string::npos ? target : target.substr(0, queryStart);
  request.path = urlDecode(path.empty() ? "/" : path);

  if (queryStart != std::string::npos) {
    std::istringstream queryStream(target.substr(queryStart + 1));
    std::string pair;
    while (std::getline(queryStream, pair, '&')) {
      auto equals = pair.find('=');
      auto key = equals == std::string::npos ? pair : pair.substr(0, equals);
      auto value = equals == std::string::npos ? "" : pair.substr(equals + 1);
      request.query[urlDecode(key)] = urlDecode(value);
    }
  }

  std::string header;
  while (std::getline(stream, header)) {
    if (!header.empty() && header.back() == '\r') header.pop_back();
    auto separator = header.find(':');
    if (separator != std::string::npos) {
      auto key = header.substr(0, separator);
      auto value = header.substr(separator + 1);
      while (!value.empty() && value.front() == ' ') value.erase(value.begin());
      request.headers[key] = value;
    }
  }

  request.body = raw.substr(headerEnd + 4);
  return true;
}

bool serveStatic(const HttpRequest& request, HttpResponse& response) {
  for (const auto& route : staticRoutes) {
    if (request.path.rfind(route.prefix, 0) != 0) continue;

    auto relative = request.path.substr(route.prefix.size());
    if (relative.empty() || relative == "/") relative = "/index.html";
    auto requested = route.root / relative.substr(1);

    std::error_code error;
    auto root = std::filesystem::weakly_canonical(route.root, error);
    auto file = std::filesystem::weakly_canonical(requested, error);
    auto rootString = root.string();
    auto fileString = file.string();
    auto withinRoot = file == root || rootString == "/" || fileString.rfind(rootString + "/", 0) == 0;
    if (error || !withinRoot) {
      return false;
    }

    if (!std::filesystem::is_regular_file(file, error) || error) return false;
    std::ifstream input(file, std::ios::binary);
    if (!input) return false;
    response.contentType = contentType(file);
    response.body.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return true;
  }
  return false;
}

void sendResponse(int client, const HttpResponse& response) {
  std::ostringstream output;
  output << "HTTP/1.1 " << response.status << " " << statusText(response.status) << "\r\n";
  output << "Content-Type: " << response.contentType << "\r\n";
  output << "Content-Length: " << response.body.size() << "\r\n";
  output << "Connection: close\r\n";
  for (const auto& header : response.headers) {
    output << header.first << ": " << header.second << "\r\n";
  }
  output << "\r\n" << response.body;

  auto data = output.str();
  send(client, data.data(), data.size(), MSG_NOSIGNAL);
}

bool handleClient(Client& client) {
  char buffer[16384];
  auto bytesRead = recv(client.fd, buffer, sizeof(buffer), 0);
  if (bytesRead == 0) return true;
  if (bytesRead < 0) {
    return errno != EAGAIN && errno != EWOULDBLOCK;
  }

  client.data.append(buffer, static_cast<size_t>(bytesRead));
  if (client.data.size() > 16384) {
    HttpResponse response;
    response.status = 400;
    response.body = "Bad Request\n";
    sendResponse(client.fd, response);
    return true;
  }

  if (client.data.find("\r\n\r\n") == std::string::npos) return false;

  HttpRequest request;
  HttpResponse response;
  if (!parseRequest(client.data, request)) {
    response.status = 400;
    response.body = "Bad Request\n";
    sendResponse(client.fd, response);
    return true;
  }

  auto contentLength = request.headers.find("Content-Length");
  if (contentLength != request.headers.end()) {
    char* end = nullptr;
    auto expected = std::strtoul(contentLength->second.c_str(), &end, 10);
    if (*end != '\0' || request.body.size() < expected) return false;
  }

  bool handled = false;
  for (const auto& route : routes) {
    if (route.method == request.method && route.path == request.path) {
      route.handler(request, response);
      handled = true;
      break;
    }
  }

  if (handled) {
    sendResponse(client.fd, response);
    return true;
  }

  if (request.method != "GET") {
    response.status = 405;
    response.headers["Allow"] = "GET, POST";
    response.body = "Method Not Allowed\n";
    sendResponse(client.fd, response);
    return true;
  }

  if (!handled) handled = serveStatic(request, response);
  if (!handled) {
    response.status = 404;
    response.body = "Not Found\n";
  }
  sendResponse(client.fd, response);
  return true;
}

}

void httpGet(const std::string& path, HttpHandler handler) {
  routes.push_back({"GET", path, std::move(handler)});
}

void httpPost(const std::string& path, HttpHandler handler) {
  routes.push_back({"POST", path, std::move(handler)});
}

void httpStatic(const std::string& urlPrefix, const std::string& root) {
  staticRoutes.push_back({urlPrefix, std::filesystem::path(root)});
}

bool httpServerStart(const std::string& endpoint) {
  auto separator = endpoint.rfind(':');
  auto host = separator == std::string::npos ? "127.0.0.1" : endpoint.substr(0, separator);
  auto portText = separator == std::string::npos ? endpoint : endpoint.substr(separator + 1);
  char* portEnd = nullptr;
  auto port = std::strtol(portText.c_str(), &portEnd, 10);
  if (*portEnd != '\0' || port < 1 || port > 65535) {
    std::cerr << "http server: invalid port in " << endpoint << std::endl;
    return false;
  }

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    std::cerr << "http server: socket creation failed: " << std::strerror(errno) << std::endl;
    return false;
  }

  int enabled = 1;
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

  sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_port = htons(static_cast<uint16_t>(port));
  address.sin_addr.s_addr = host == "0.0.0.0" ? htonl(INADDR_ANY) : inet_addr(host.c_str());
  if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 ||
      listen(serverSocket, 16) < 0 ||
      !setNonBlocking(serverSocket)) {
    std::cerr << "http server: failed to listen on " << endpoint << ": " << std::strerror(errno) << std::endl;
    close(serverSocket);
    serverSocket = -1;
    return false;
  }

  std::cout << "http server: listening on " << endpoint << std::endl;
  return true;
}

void httpServerUpdate() {
  if (serverSocket < 0) return;

  while (true) {
    auto client = accept(serverSocket, nullptr, nullptr);
    if (client < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) break;
      break;
    }
    if (!setNonBlocking(client)) {
      close(client);
      continue;
    }
    clients.push_back({client, {}});
  }

  for (auto iterator = clients.begin(); iterator != clients.end();) {
    if (handleClient(*iterator)) {
      close(iterator->fd);
      iterator = clients.erase(iterator);
    } else {
      ++iterator;
    }
  }
}

void httpServerStop() {
  for (auto& client : clients) close(client.fd);
  clients.clear();
  if (serverSocket >= 0) close(serverSocket);
  serverSocket = -1;
}

void httpResponseSend(HttpResponse& response, int status, const std::string& contentType, const std::string& body) {
  response.status = status;
  response.contentType = contentType;
  response.body = body;
}

void httpResponseJson(HttpResponse& response, int status, const std::string& body) {
  httpResponseSend(response, status, "application/json", body);
}
