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
void renderActiveScene(bool includePanel, std::optional<objid> activeScene);
void renderCreateObj(bool includePanel, std::optional<objid> activeScene);

void renderCameraPanel(bool includePanel);
void renderLightPanel(bool includePanel);
void renderMeshPanel(bool includePanel, std::optional<objid> objectToDetail);
void renderUnknownObjPanel(bool includePanel);
void renderObjPanel(bool includePanel, std::optional<objid> objectToDetail);

void renderBallGameplay(bool includePanel);

void renderObjectDetails(objid id, bool includePanel);
void renderRenderPanel(bool includePanel);
void renderTransformPanel(bool includePanel);

#endif