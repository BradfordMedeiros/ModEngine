#include "./core.h"

#include <filesystem>
#include <set>
#include <iostream>
#include <optional>
#include "../../common/util.h"

extern CustomApiBindings* mainApi;

typedef int32_t objid;
objid rootObjId();
std::set<objid> childObj(objid id);
std::optional<std::string> getGameObjectName(int32_t index);
void removeObjectById(objid id);
void moveCameraAbs(glm::vec3 position);


std::optional<objid> ScenegraphView2(objid id){
    std::optional<objid> selectedId;

    auto children = childObj(id);

    std::cout << "scenegraph: " << id << ", size = " << children.size() << std::endl;

    if (children.size() > 0){
        bool selected = true;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        if (ImGui::TreeNodeEx(getGameObjectName(id).value().c_str(), flags))
        {
            for (auto childId : children){
                auto selectedObjId = ScenegraphView2(childId);
                if (selectedObjId.has_value()){
                    selectedId = selectedObjId;
                }

            }
            ImGui::TreePop();
        }
    }else{
            if(ImGui::Selectable(getGameObjectName(id).value().c_str())){
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


std::optional<objid> renderScenegraph(const char* name){
    ImGui::Begin(name, nullptr);
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("Assets", ImVec2(size.x, size.y), true);
  
    auto selectedId = ScenegraphView2(rootObjId());
    if (selectedId.has_value()){
        std::cout << "scenegraph selected: " << selectedId.value() << std::endl;
    }

    ImGui::EndChild();
    ImGui::End();

    return selectedId;
}


