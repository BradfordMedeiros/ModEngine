#include "./core.h"

#include <filesystem>
#include <set>
#include <iostream>
#include <optional>
#include "../../common/util.h"

extern CustomApiBindings* mainApi;
extern DefaultResources defaultResources;
extern Stats statistics;
extern engineState state;

typedef int32_t objid;
objid rootObjId();
std::set<objid> childObj(objid id);
std::optional<std::string> getGameObjectName(int32_t index);
void removeObjectById(objid id);
void moveCameraAbs(glm::vec3 position);
void sendManipulatorEvent(MANIPULATOR_EVENT event);
std::optional<std::string> ScenegraphView(std::string directory, FILE_EXTENSION_TYPE type);

std::optional<objid> ScenegraphView2(objid id, std::optional<objid> lastSelectedId){
    std::optional<objid> selectedId;

    auto children = childObj(id);

    std::cout << "scenegraph: " << id << ", size = " << children.size() << std::endl;

    bool selected = lastSelectedId.has_value() && lastSelectedId.value() == id;
    if (children.size() > 0){
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        if (ImGui::TreeNodeEx(getGameObjectName(id).value().c_str(), flags))
        {
            for (auto childId : children){
                auto selectedObjId = ScenegraphView2(childId, lastSelectedId);
                if (selectedObjId.has_value()){
                    selectedId = selectedObjId;
                }

            }
            ImGui::TreePop();
        }
    }else{
            if(ImGui::Selectable(getGameObjectName(id).value().c_str(), selected)){
                selectedId = id;
            }

            if (ImGui::BeginPopupContextItem()){
                if (ImGui::MenuItem("Go To"))
                {   
                    auto objectPos = mainApi -> getGameObjectPos(id, true, "scenegraph pos");
                    auto orientation = mainApi -> orientationFromPos(objectPos, objectPos + glm::vec3(0.f, 0.f, -1.f));
                    moveCameraAbs(objectPos + orientation * glm::vec3(0.f, 0.f, 5.f));
                }
                if (ImGui::MenuItem("Delete"))
                {
                    mainApi -> removeObjectById(id);
                }
            
                ImGui::EndPopup();
            }
    }

    return selectedId;
    
}


std::optional<objid> renderScenegraph(const char* name, bool includePanel, std::optional<objid> selectedObjId){
    if (includePanel){
        ImGui::Begin(name, nullptr);
    }
    ImVec2 size = ImGui::GetContentRegionAvail();
  
    auto selectedId = ScenegraphView2(rootObjId(), selectedObjId);
    if (selectedId.has_value()){
        std::cout << "scenegraph selected: " << selectedId.value() << std::endl;
    }

    if (includePanel){
        ImGui::End();
    }

    return selectedId;
}


void renderDebug(bool includePanel){
    if (includePanel){
      ImGui::Begin("Debug Panel");
    }

  {
    auto currValue = isEditorDebug();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Debug", &currValue);
    if(oldValue != currValue){
      setEditorDebug(currValue);
    }
  }

  {
    auto currValue = isShowCamera();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Cameras", &currValue);
    if(oldValue != currValue){
      setShowCamera(currValue);
    }
  }

  {
    auto currValue = isShowSound();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Sounds", &currValue);
    if(oldValue != currValue){
      setShowSound(currValue);
    }
  }
  {
    auto currValue = isShowEmitters();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Emitters", &currValue);
    if(oldValue != currValue){
      setShowEmitters(currValue);
    }
  }

  {
    auto currValue = isShowLights();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Lights", &currValue);
    if(oldValue != currValue){
      setShowLights(currValue);
    }
  }


  {
    auto currValue = isVisualizeVoxelLighting();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Grid", &currValue);
    if(oldValue != currValue){
      setVisualizeVoxelLighting(currValue);
    }
  }



  ImGui::Dummy(ImVec2(0, 10));

  {
    auto currValue = isMuted();
    auto oldValue = currValue;
    ImGui::Checkbox("Mute", &currValue);
    if(oldValue != currValue){
      setIsMuted(currValue);
    }
  }

  if (includePanel){
      ImGui::End();
  }
}


void renderCoreDebug(bool includePanel){
  if (includePanel){
    ImGui::Begin("CoreDebug");
  }

  {
    ImGui::Text("Framerate: ");
    ImGui::SameLine();
    ImGui::Text("%i", static_cast<int>(unwrapStat<float>(statValue(statistics.fpsStat))));
  }

  {
    ImGui::Text("Selected Name: ");
    ImGui::SameLine();

    auto ids = state.editor.selectedObjs;
    if (ids.size() > 0 && gameobjExists(ids.at(0))){
      auto selectedObject = mainApi -> getGameObjNameForId(ids.at(0)).value();
      ImGui::Text("%s", selectedObject.c_str());
    }else{
      ImGui::Text("[none]");

    }
  }


  {
    ImGui::Text("Default Camera Position: ");
    ImGui::SameLine();
    ImGui::Text("%s", print(defaultResources.defaultCamera.transformation.position).c_str());
  }
  {
    ImGui::Text("Default Camera Rotation: ");
    ImGui::SameLine();
    ImGui::Text("%s", print(defaultResources.defaultCamera.transformation.rotation).c_str());
  }

  {
    float ndiX = 2 * (state.cursorLeft / (float)state.resolution.x) - 1.f;
    float ndiY = -2 * (state.cursorTop / (float)state.resolution.y) + 1.f;
    ImGui::Text("Cursor NDI: ");
    ImGui::SameLine();
    ImGui::Text("%s", print(glm::vec2(ndiX, ndiY)).c_str());
  }

  {
    auto idExists = gameobjExists(state.currentHoverIndex);
    std::string name = idExists ? getGameObjectName(state.currentHoverIndex).value() : "[none]";

    ImGui::Text("hovered id: ");
    ImGui::SameLine();
    ImGui::Text("%d", state.currentHoverIndex);
    ImGui::SameLine();
    ImGui::Text("%s", name.c_str());
  }
  {
    ImGui::Text("triangles");
    ImGui::SameLine();
    ImGui::Text("%d", statistics.numTriangles);
  }

  {
    ImGui::Text("draw calls");
    ImGui::SameLine();
    ImGui::Text("%d", statistics.numDrawCalls);
  }

  {
    ImGui::Text("num gameobjects");
    ImGui::SameLine();
    ImGui::Text("%d", unwrapStat<int>(statValue(statistics.numObjectsStat)));
  }

  {
    ImGui::Text("num rigidbodys");
    ImGui::SameLine();
    ImGui::Text("%d", unwrapStat<int>(statValue(statistics.rigidBodiesStat)));
  }


  {
    ImGui::Text("num scenes loaded");
    ImGui::SameLine();
    ImGui::Text("%d",unwrapStat<int>(statValue(statistics.scenesLoadedStat)));
  }


  {
    ImGui::Text("time");
    ImGui::SameLine();
    ImGui::Text("%f", timeSeconds(false));
  }

  {
    ImGui::Text("realtime");
    ImGui::SameLine();
    ImGui::Text("%f", timeSeconds(true));
  }

  if (includePanel){
    ImGui::End();
  }    
}

void renderObjectCount(bool includePanel){
    struct ObjectCount {
        std::string field;
        std::string statName;
    };

    std::vector<ObjectCount> objects {
        ObjectCount {
            .field = "Object count",
            .statName = "object-count",
        },
        ObjectCount {
            .field = "Rigid bodies",
            .statName = "rigidbody-count",
        },
        ObjectCount {
            .field = "Scenes loaded",
            .statName = "scenes-loaded",
        },
        ObjectCount {
            .field = "Num Textures",
            .statName = "num-textures",
        },
        ObjectCount {
            .field = "Num Models",
            .statName = "num-models",
        },
        ObjectCount {
            .field = "Num Meshes",
            .statName = "num-meshes",
        },
        ObjectCount {
            .field = "Num Animations",
            .statName = "num-animations",
        },
    };


    if (includePanel){
        ImGui::Begin("Object Count");
    }

    for (auto& object : objects){
        auto statSymbol = mainApi -> stat(object.statName);
        auto statValue = mainApi -> statValue(statSymbol);
        auto objectCount = std::get_if<int>(&statValue);
        modassert(objectCount, "object count is NULL");

        ImGui::Text(object.field.c_str());
        ImGui::SameLine();
        ImGui::Text(std::to_string(*objectCount).c_str());
    }

    if (includePanel){
        ImGui::End();
    }
}

void renderActiveScene(bool includePanel, std::optional<objid> activeScene){
    if (includePanel){
      ImGui::Begin("Active Scene");
    }

  if (activeScene.has_value()){
    ImGui::Text((std::string("Active Id = ") + std::to_string(activeScene.value())).c_str());
    if(ImGui::Button("Save Scene")){
      mainApi -> saveScene(false /*include ids */, activeScene.value(), std::nullopt /* filename */);
    }
    if(ImGui::Button("Reset Scene")){
      mainApi -> resetScene(activeScene.value());
    }
    if(ImGui::Button("New Scene")){
      modassert(false, "not yet implemented");
    }
    if(ImGui::Button("Load Scene")){
      ImGui::OpenPopup("load-scene-modal");
    }

    {
      if (ImGui::BeginPopupModal("load-scene-modal")){
        auto selectedScene = ScenegraphView("../afterworld/scenes/levels/worlds", RAWSCENE_EXTENSION);
        if (selectedScene.has_value()){
          std::cout << "load scene: " << selectedScene.value() << std::endl;

          if (activeScene.has_value()){
            mainApi -> unloadScene(activeScene.value());
          }

          mainApi -> loadScene(selectedScene.value(), {}, std::nullopt, std::nullopt);

        }
          /*std::string name = "";
          ImGui::InputText("Name", &name);
          if (ImGui::Button("OK"))
          {
              std::cout << "create weapon: " << name << std::endl;
      
              ImGui::CloseCurrentPopup();
          }
      
          ImGui::SameLine();
      
          if (ImGui::Button("Cancel"))
          {
              ImGui::CloseCurrentPopup();
          }*/
      
          ImGui::EndPopup();
      }
    }

  }

  if (includePanel){
      ImGui::End();
  }
}