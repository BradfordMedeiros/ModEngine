#include "./merge_gameobjects.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "../../state.h"
#include "../../common/util.h"

extern World world;
extern engineState state;

void renderGameObjectMergeWidget(bool includePanel, std::optional<objid> sceneId){
  static std::string outputPath = "./build/merged_gameobjects.modelb";
  static std::string status;
  static bool lastMergeSucceeded = false;
  static bool mergeAllSceneCandidates = false;
  static std::vector<objid> sceneCandidates;
  static objid candidatesForScene = -1;
  static size_t skippedSceneObjects = 0;

  if (includePanel){
    ImGui::Begin("Merge Game Objects");
  }

  auto& selectedIds = state.editor.selectedObjs;
  if (ImGui::RadioButton("Merge selected", !mergeAllSceneCandidates)){
    mergeAllSceneCandidates = false;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Merge all scene candidates", mergeAllSceneCandidates)){
    mergeAllSceneCandidates = true;
  }

  std::vector<objid> idsToMerge;
  if (mergeAllSceneCandidates){
    if (!sceneId.has_value()){
      ImGui::TextUnformatted("Select an object in the active scene first.");
      sceneCandidates.clear();
      candidatesForScene = -1;
    }else{
      ImGui::Text("Active scene ID: %d", sceneId.value());
      if (candidatesForScene != sceneId.value()){
        sceneCandidates.clear();
        candidatesForScene = -1;
      }
      if (ImGui::Button("Find candidates")){
        sceneCandidates.clear();
        skippedSceneObjects = 0;
        for (objid id : listObjInScene(world.sandbox, sceneId)){
          if (getGameObjectH(world.sandbox, id).parentId != 0){
            continue;
          }
          auto candidate = tryMergeGameObjects(world, { id }, true);
          if (candidate.eligible){
            sceneCandidates.push_back(id);
          }else{
            skippedSceneObjects++;
          }
        }
        candidatesForScene = sceneId.value();
        status = "Found " + std::to_string(sceneCandidates.size()) +
          " merge candidates; skipped " + std::to_string(skippedSceneObjects) + " other scene roots.";
        lastMergeSucceeded = true;
      }
      idsToMerge = sceneCandidates;
      ImGui::Text("Scene candidates: %zu (skipped %zu roots)", sceneCandidates.size(), skippedSceneObjects);
    }
  }else{
    idsToMerge = selectedIds;
    ImGui::Text("Selected objects: %zu", selectedIds.size());
  }

  for (objid id : idsToMerge){
    if (idExists(world.sandbox, id)){
      ImGui::BulletText("%s (%d)", getGameObject(world.sandbox, id).name.c_str(), id);
    }
  }

  ImGui::InputText("Output model", &outputPath);
  auto extension = getExtension(outputPath);
  bool validOutputPath = !outputPath.empty() && extension.has_value() &&
    (extension.value() == "model" || extension.value() == "modelb");
  if (!validOutputPath){
    ImGui::TextUnformatted("Output path must end in .model or .modelb.");
  }

  bool canMerge = idsToMerge.size() >= 2 && validOutputPath &&
    (!mergeAllSceneCandidates || (sceneId.has_value() && candidatesForScene == sceneId.value()));
  ImGui::BeginDisabled(!canMerge);
  bool mergeClicked = ImGui::Button("Merge & Save");
  ImGui::EndDisabled();

  if (mergeClicked){
    auto result = tryMergeGameObjects(world, idsToMerge);
    if (!result.modelData.has_value()){
      status = result.refusalReason;
      lastMergeSucceeded = false;
    }else{
      saveModelData(world, result.modelData.value(), outputPath);
      status = "Merged model saved to " + outputPath + " (" + std::to_string(result.modelData->meshIdToMeshData.size()) + " meshes).";
      lastMergeSucceeded = true;
    }
  }

  if (!status.empty()){
    ImGui::TextColored(lastMergeSucceeded ? ImVec4(0.3f, 0.9f, 0.3f, 1.f) : ImVec4(1.f, 0.4f, 0.3f, 1.f), "%s", status.c_str());
  }

  if (includePanel){
    ImGui::End();
  }
}
