#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "../../cscript/cscript_binding.h"
#include "../../object_util.h"

void renderCameraPanel(bool includePanel);
void renderSoundPanel(bool includePanel, std::optional<objid> objectToDetail);
void renderLightPanel(bool includePanel, std::optional<objid> objectToDetail);
void renderMeshPanel(bool includePanel, std::optional<objid> objectToDetail);
void renderParticlePanel(bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId);
void renderUnknownObjPanel(bool includePanel);
void renderObjPanel(bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId);

void renderObjectDetails(objid id, bool includePanel);


glm::vec3 createLocation();
void renderCreateObj(bool includePanel, std::optional<objid> activeScene);
void renderModelPanel(bool includePanel, std::optional<objid> sceneId);
