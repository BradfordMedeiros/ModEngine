#include "./index.h"

#include "../network/http_server.h"
#include "../package.h"
#include "./package_report.h"
#include "./playlist_editor.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

bool setupHttpServer(const std::string& dataPath, bool packageMounted){
   if (!httpServerStart("127.0.0.1:8085")){
      return false;
   }

   auto packageReportPath = dataPath + "http-package-report.html";
   httpGet("/package", [packageReportPath, packageMounted](const HttpRequest&, HttpResponse& response) {
      if (!packageMounted){
         httpResponseSend(response, 200, "text/plain", "Package not mounted\n");
         return;
      }

      if (!packageSizeReport(packageReportPath.c_str())){
         httpResponseSend(response, 500, "text/plain", "Could not create package report\n");
         return;
      }

      std::ifstream report(packageReportPath, std::ios::binary);
      if (!report){
         httpResponseSend(response, 500, "text/plain", "Could not read package report\n");
         std::remove(packageReportPath.c_str());
         return;
      }

      std::string html((std::istreambuf_iterator<char>(report)), std::istreambuf_iterator<char>());
      report.close();
      std::remove(packageReportPath.c_str());
      httpResponseSend(response, 200, "text/html", html);
   });

   httpGet("/playlists", [](const HttpRequest&, HttpResponse& response) {
      servePlaylistEditor(response);
   });
   httpGet("/api/playlists", [](const HttpRequest& request, HttpResponse& response) {
      servePlaylistApi(request, response);
   });
   httpGet("/api/levels", [](const HttpRequest& request, HttpResponse& response) {
      servePlaylistApi(request, response);
   });
   httpGet("/api/playlist", [](const HttpRequest& request, HttpResponse& response) {
      servePlaylistApi(request, response);
   });
   httpPost("/api/playlist", [](const HttpRequest& request, HttpResponse& response) {
      servePlaylistApi(request, response);
   });
   httpGet("/api/config-levels", [](const HttpRequest& request, HttpResponse& response) {
      servePlaylistApi(request, response);
   });
   httpGet("/api/level", [](const HttpRequest& request, HttpResponse& response) {
      servePlaylistApi(request, response);
   });
   httpPost("/api/level", [](const HttpRequest& request, HttpResponse& response) {
      servePlaylistApi(request, response);
   });
   return true;
}
