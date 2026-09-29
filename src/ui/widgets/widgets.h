#ifndef MOD_GUI_WIDGETS
#define MOD_GUI_WIDGETS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "../../cscript/cscript_binding.h"
#include "../../object_util.h"
#include "../../main_api.h"
#include "./resource.h"

void renderRenderPanel(bool includePanel);
void renderTransformPanel(bool includePanel);
void renderTextures(bool includePanel, std::optional<objid> objectToDetail);
void renderDisplayBinding(bool includePanel);
void renderFontWidget(bool includePanel);
void renderFontBindingWidget(bool includePanel);
void renderColorWidget(bool includePanel);

void renderMeshPointEditorWidget(bool includePanel);
void renderOrbUiPointEditorWidget(bool includePanel, std::optional<objid> sceneId);


struct PointConfig {
  std::vector<glm::vec3> position;
  std::vector<glm::quat> rotations;
  std::vector<int> connections;
  std::vector<std::optional<std::string>> names;
  std::vector<std::optional<std::string>> orbUis;
  std::vector<std::optional<std::string>> levels;
};

PointConfig loadPointConfig(std::string filepath);
void savePointConfig(std::string filepath, PointConfig pointConfig);
std::string print(PointConfig pointConfig);

#endif