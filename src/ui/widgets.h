#ifndef MOD_GUI_WIDGETS
#define MOD_GUI_WIDGETS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "../cscript/cscript_binding.h"
#include "../object_util.h"

void renderDebug(bool includePanel);
void renderObjectCount(bool includePanel);
void renderActiveScene(bool includePanel);

void renderCameraPanel(bool includePanel);
void renderLightPanel(bool includePanel);

void renderBallGameplay(bool includePanel);

void renderObjectDetails(objid id, bool includePanel);

#endif