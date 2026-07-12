#include "./core.h"

#include <filesystem>
#include <set>
#include <iostream>
#include <optional>

typedef int32_t objid;
objid rootObjId();
std::set<objid> childObj(objid id);
std::optional<std::string> getGameObjectName(int32_t index);

void ScenegraphView(std::string directory){
    for (auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_directory())
        {
            if (ImGui::TreeNode(entry.path().filename().string().c_str()))
            {
                ScenegraphView(entry.path());
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::Selectable(
                entry.path().filename().string().c_str()
            );
        }
    }
}

void ScenegraphView2(objid id){
    auto children = childObj(id);

    std::cout << "scenegraph: " << id << ", size = " << children.size() << std::endl;

    if (children.size() > 0){
        bool selected = true;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        if (ImGui::TreeNodeEx(getGameObjectName(id).value().c_str(), flags))
        {
            for (auto childId : children){
                ScenegraphView2(childId);

            }
            ImGui::TreePop();
        }
    }else{
            ImGui::Selectable(
                getGameObjectName(id).value().c_str()
            );
    }
    
}


void renderScenegraph(const char* name){
    ImGui::Begin(name, nullptr);
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("Assets", ImVec2(size.x, size.y), true);
  
    ScenegraphView2(rootObjId());
   
    ImGui::EndChild();
    ImGui::End();
}


