#include "./playlist_editor.h"

#include "../package.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>
#include <rapidjson/document.h>

namespace {

const std::string playlistDirectory = "../afterworld/data/config/playlists";
const std::string levelDirectory = "../afterworld/scenes/levels/worlds";

std::string jsonEscape(const std::string& value){
   std::string escaped;
   for (char character : value){
      if (character == '\\') escaped += "\\\\";
      else if (character == '"') escaped += "\\\"";
      else if (character == '\n') escaped += "\\n";
      else if (character == '\r') escaped += "\\r";
      else escaped += character;
   }
   return escaped;
}

std::vector<std::string> playlistFiles(){
   auto files = listFilesWithExtensionsFromPackage(playlistDirectory, {"json"});
   std::sort(files.begin(), files.end());
   return files;
}

std::vector<std::pair<std::string, std::string>> availableLevels(){
   auto files = listFilesWithExtensionsFromPackage(levelDirectory, {"rawscene"});
   std::vector<std::pair<std::string, std::string>> levels;
   for (const auto& file : files){
      auto path = std::filesystem::path(file);
      if (path.filename() != "map.rawscene") continue;
      auto level = path.parent_path().filename().string();
      auto world = path.parent_path().parent_path().filename().string();
      if (!world.empty() && !level.empty()){
         levels.push_back({world, level});
      }
   }
   std::sort(levels.begin(), levels.end());
   levels.erase(std::unique(levels.begin(), levels.end()), levels.end());
   return levels;
}

std::vector<std::string> levelConfigFiles(){
   auto files = listFilesWithExtensionsFromPackage(levelDirectory, {"json"});
   files.erase(std::remove_if(files.begin(), files.end(), [](const std::string& file) {
      return std::filesystem::path(file).filename() != "config.json";
   }), files.end());
   std::sort(files.begin(), files.end());
   return files;
}

bool validDocument(const std::string& content, bool requireLevels){
   rapidjson::Document document;
   if (document.Parse(content.c_str()).HasParseError() || !document.IsObject()){
      return false;
   }
   return !requireLevels || (document.HasMember("levels") && document["levels"].IsArray());
}

bool writeNativeResource(const std::string& path, const std::string& content, const std::vector<std::string>& allowedFiles){
   if (std::find(allowedFiles.begin(), allowedFiles.end(), path) == allowedFiles.end()){
      return false;
   }
   if (!std::filesystem::is_regular_file(path)){
      return false;
   }
   std::ofstream output(path, std::ios::binary | std::ios::trunc);
   if (!output) return false;
   output << content;
   if (!content.empty() && content.back() != '\n') output << '\n';
   return output.good();
}

}

bool servePlaylistEditor(HttpResponse& response){
   std::ifstream page("./src/ui_html/playlist_editor.html", std::ios::binary);
   if (!page){
      httpResponseSend(response, 500, "text/plain", "Playlist editor is unavailable\n");
      return false;
   }

   std::string html((std::istreambuf_iterator<char>(page)), std::istreambuf_iterator<char>());
   httpResponseSend(response, 200, "text/html; charset=utf-8", html);
   return true;
}

bool servePlaylistApi(const HttpRequest& request, HttpResponse& response){
   if (request.path == "/api/playlists"){
      auto files = playlistFiles();
      std::string body = "{\"playlists\":[";
      for (size_t i = 0; i < files.size(); i++){
         if (i > 0) body += ",";
         auto name = std::filesystem::path(files[i]).stem().string();
         body += "{\"name\":\"" + jsonEscape(name) + "\",\"path\":\"" + jsonEscape(files[i]) + "\"}";
      }
      body += "]}";
      httpResponseJson(response, 200, body);
      return true;
   }

   if (request.path == "/api/levels"){
      auto levels = availableLevels();
      std::string body = "{\"levels\":[";
      for (size_t i = 0; i < levels.size(); i++){
         if (i > 0) body += ",";
         body += "{\"world\":\"" + jsonEscape(levels[i].first) + "\",\"level\":\"" + jsonEscape(levels[i].second) + "\"}";
      }
      body += "]}";
      httpResponseJson(response, 200, body);
      return true;
   }

   if (request.path == "/api/config-levels"){
      auto files = levelConfigFiles();
      std::string body = "{\"levels\":[";
      for (size_t i = 0; i < files.size(); i++){
         if (i > 0) body += ",";
         auto path = std::filesystem::path(files[i]);
         auto level = path.parent_path().filename().string();
         auto world = path.parent_path().parent_path().filename().string();
         body += "{\"world\":\"" + jsonEscape(world) + "\",\"level\":\"" + jsonEscape(level)
               + "\",\"path\":\"" + jsonEscape(files[i]) + "\"}";
      }
      body += "]}";
      httpResponseJson(response, 200, body);
      return true;
   }

   if (request.path == "/api/playlist"){
      auto path = request.query.find("path");
      auto files = playlistFiles();
      if (path == request.query.end()){
         httpResponseSend(response, 404, "text/plain", "Playlist not found\n");
         return true;
      }
      if (request.method == "POST"){
         if (!validDocument(request.body, true)){
            httpResponseSend(response, 400, "text/plain", "Invalid playlist data\n");
         }else if (!writeNativeResource(path->second, request.body, files)){
            httpResponseSend(response, 409, "text/plain", "Mounted packages cannot be edited\n");
         }else{
            httpResponseSend(response, 200, "text/plain", "Playlist saved\n");
         }
         return true;
      }
      if (std::find(files.begin(), files.end(), path->second) == files.end()){
         httpResponseSend(response, 404, "text/plain", "Playlist not found\n");
         return true;
      }
      auto content = readFileOrPackage(path->second);
      httpResponseSend(response, 200, "application/json", content);
      return true;
   }

   if (request.path == "/api/level"){
      auto path = request.query.find("path");
      auto files = levelConfigFiles();
      if (path == request.query.end()){
         httpResponseSend(response, 404, "text/plain", "Level not found\n");
         return true;
      }
      if (request.method == "POST"){
         if (!validDocument(request.body, false)){
            httpResponseSend(response, 400, "text/plain", "Invalid level data\n");
         }else if (!writeNativeResource(path->second, request.body, files)){
            httpResponseSend(response, 409, "text/plain", "Mounted packages cannot be edited\n");
         }else{
            httpResponseSend(response, 200, "text/plain", "Level saved\n");
         }
         return true;
      }
      if (std::find(files.begin(), files.end(), path->second) == files.end()){
         httpResponseSend(response, 404, "text/plain", "Level not found\n");
         return true;
      }
      auto content = readFileOrPackage(path->second);
      httpResponseSend(response, 200, "application/json", content);
      return true;
   }

   return false;
}
