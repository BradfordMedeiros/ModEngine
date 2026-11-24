#ifndef MOD_UI_HTML_PLAYLIST_EDITOR
#define MOD_UI_HTML_PLAYLIST_EDITOR

#include "../network/http_server.h"

bool servePlaylistEditor(HttpResponse& response);
bool servePlaylistApi(const HttpRequest& request, HttpResponse& response);

#endif
