#include "./gui.h"

#ifndef USE_IMGUI

void initUi(){}
void renderUi(){}
void registerWidget(std::string name, std::string list, std::function<void(bool includePanel,  std::optional<objid> objectToDetail, std::optional<objid> sceneId)> render){}
void registerAction(std::string name, std::string list, std::function<void()> fn){}

#else 

extern CustomApiBindings* mainApi;
extern GLFWwindow* window;

void initUi(){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

    registerWidget("Debug", "default", [](bool includePanel, std::optional<objid>, std::optional<objid>) -> void {
        renderDebug(includePanel);
    });
    registerWidget("Transform", "default", [](bool includePanel, std::optional<objid>, std::optional<objid>) -> void {
        renderTransformPanel(includePanel);
    });
    registerWidget("ActiveScene", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderActiveScene(includePanel, sceneId);
    });
    registerWidget("Create Obj", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderCreateObj(includePanel, sceneId);
    });
    
    registerWidget("Render", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderRenderPanel(includePanel);
    });

    registerWidget("Object Count", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderObjectCount(includePanel);
    });

    registerWidget("Textures", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderTextures(includePanel, objectToDetail);
    });

    registerWidget("Model", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderModelPanel(includePanel, sceneId);
    });   
    
    registerWidget("Object - Camera", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderCameraPanel(includePanel);
    });  

    registerWidget("Object - Light", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderLightPanel(includePanel);
    });  

    registerWidget("Object - Mesh", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderMeshPanel(includePanel, objectToDetail);
    });  

    registerWidget("Particle", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderParticlePanel(includePanel, objectToDetail, sceneId);
    });  
           
}

void renderConsole(){
ImGui::Begin("Console");

ImGui::BeginChild("Log",
                  ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                  true,
                  ImGuiWindowFlags_HorizontalScrollbar);

//for (const LogEntry& entry : log)
//{
//    ImGui::TextUnformatted(entry.text.c_str());
//}

static float fpsHistory[120] = { 0 };
static int offsetV = 0;

fpsHistory[offsetV] = 20;

// Every frame
offsetV = (offsetV + 1) % 120;

ImGui::PlotLines(
    "FPS",
    fpsHistory,
    IM_ARRAYSIZE(fpsHistory),
    offsetV,          // ring buffer offset
    nullptr,         // overlay text
    0.0f,
    240.0f,
    ImVec2(0,80));

ImGui::EndChild();

// Input line
static std::string testname1 = "hello world";
ImGui::InputText("##input", &testname1);


ImGui::End();

}

struct RegisteredAction {
    std::string name;
    std::function<void()> fn;
};

struct RegisteredActions {
    std::string list;
    std::vector<RegisteredAction> actions;
};

std::vector<RegisteredActions> registeredActionLists;

void registerAction(std::string name, std::string list, std::function<void()> fn){
    bool foundList = false;
    for (auto& registeredActionList : registeredActionLists){
        if (registeredActionList.list == list){
            registeredActionList.actions.push_back(RegisteredAction {
                .name = name,
                .fn = fn,
            });
            return;
        }
    }
    registeredActionLists.push_back(RegisteredActions{
        .list = list,
        .actions  = {
            RegisteredAction {
                .name = name,
                .fn = fn,
            }
        }
    });

}

enum ImMenuView { MENUVIEW_NONE, MENUVIEW_EDITOR };
ImMenuView menuViewState = MENUVIEW_NONE;

struct WidgetMenuItem2 {
    int id;
    std::string name;
    std::string list;
    std::function<void(bool includePanel,  std::optional<objid> objectToDetail, std::optional<objid> sceneId)> render;
};
std::vector<WidgetMenuItem2> dynamicWidgets {};
std::vector<std::string> widgetLists;
std::set<int> dynamicWidgetEnabled;

void registerWidget(std::string name, std::string list, std::function<void(bool includePanel,  std::optional<objid> objectToDetail, std::optional<objid> sceneId)> render){
    static int id = 0;
    id++;
    dynamicWidgets.push_back(WidgetMenuItem2 {
        .id = id,
        .name = name,
        .list = list,
        .render = render,
    });

    bool alreadyHasList = false;
    for (auto& widgetList : widgetLists){
        if (widgetList == list){
            alreadyHasList = true;
            break;
        }
    }
    if (!alreadyHasList){
        widgetLists.push_back(list);
    }
}
std::optional<WidgetMenuItem2*> widgetByName(std::string name){
    for (auto& widget : dynamicWidgets){
        if (name == widget.name){
            return &widget;
        }
    }
    return std::nullopt;
}


std::optional<objid> currSceneId(){
    static std::optional<objid> sceneId;
    auto sceneIdValue = activeSceneId();
    if (sceneIdValue.has_value()){
        sceneId = sceneIdValue;
    }
    return sceneId;  
}

void renderNavbar(){
    if (ImGui::BeginMainMenuBar())
    {
        // File menu
        if (ImGui::BeginMenu("File"))
        {

            if (ImGui::MenuItem("Exit"))
            {
                exit(0);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            bool showNone = menuViewState == MENUVIEW_NONE;
            bool showEditor = menuViewState == MENUVIEW_EDITOR;

            if(ImGui::MenuItem("None", nullptr, showNone)){
            	menuViewState = MENUVIEW_NONE;
            }
            if(ImGui::MenuItem("Editor", nullptr, showEditor)){
            	menuViewState = MENUVIEW_EDITOR;
            }

            ImGui::EndMenu();
        }

        for (auto& widgetList : widgetLists){
            if (ImGui::BeginMenu(widgetList.c_str())){
                if(ImGui::MenuItem("Hide All")){
                    dynamicWidgetEnabled = {};
                }
                ImGui::Separator();
                for (auto& widgetMenuItem : dynamicWidgets){
                    if (widgetMenuItem.list != widgetList){
                        continue;
                    }
                    if (ImGui::MenuItem(widgetMenuItem.name.c_str())){
                        if (dynamicWidgetEnabled.count(widgetMenuItem.id) > 0){
                            dynamicWidgetEnabled.erase(widgetMenuItem.id);
                        }else{
                            dynamicWidgetEnabled.insert(widgetMenuItem.id);
                        }
                    }
                }
                ImGui::EndMenu();
            }   
        }


        for (auto &registeredActionList : registeredActionLists){
            if (ImGui::BeginMenu(registeredActionList.list.c_str())){
                for (auto &action : registeredActionList.actions){
                    if(ImGui::MenuItem(action.name.c_str())){
                        action.fn();
                    }
                }
                ImGui::EndMenu();
            }
        }

        ImGui::EndMainMenuBar();
    }
}




void DrawDockSpace()
{	
	ImGui::Begin("TestTabs");

    if(ImGui::BeginTabBar("tabs")){
    	if (ImGui::BeginTabItem("tab1")){
   			ImGui::Text("Hello World 1");
			ImGui::EndTabItem();
    	}
    	if (ImGui::BeginTabItem("tab2")){
   			ImGui::Text("Hello World 2");
			ImGui::EndTabItem();
    	}

  	    ImGui::EndTabBar();     

    }
	ImGui::End();

}

bool Splitter(bool vertical, float thickness, float* size)
{
    ImGui::Button(
        "splitter",
        vertical ?
            ImVec2(thickness, ImGui::GetContentRegionAvail().y) :
            ImVec2(ImGui::GetContentRegionAvail().x, thickness));

    if (ImGui::IsItemActive())
    {
        *size += vertical ?
            ImGui::GetIO().MouseDelta.x :
            ImGui::GetIO().MouseDelta.y;

        return true;
    }

    return false;
}


void RenderEditor(){
    ImGui::Begin("Editor");

    static float leftWidth = 250.0f;

    ImVec2 available = ImGui::GetContentRegionAvail();

    // Left panel
    ImGui::BeginChild(
        "LeftPanel",
        ImVec2(leftWidth, available.y),
        true
    );

    ImGui::Text("Scene Hierarchy");
    ImGui::Text("Player");
    ImGui::Text("Enemy");

    ImGui::EndChild();


    ImGui::SameLine();


    // Splitter
    Splitter(true, 5.0f, &leftWidth);


    ImGui::SameLine();


    // Right panel
    ImGui::BeginChild(
        "RightPanel",
        ImVec2(0, available.y),
        true
    );

    ImGui::Text("Viewport / Inspector");
    
    ImGui::EndChild();


    ImGui::End();
}

#include <filesystem>

void FileExplorer(std::string directory){

    for (auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_directory())
        {
            if (ImGui::TreeNode(entry.path().filename().string().c_str()))
            {
                FileExplorer(entry.path());
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

std::optional<objid> getGameObjectForDetail(){
    static std::optional<objid> objectToDetail2;

    auto selectedId = mainApi -> selected();
    if (selectedId.size() > 0){
        objectToDetail2 = selectedId.at(0);
    }

    if (objectToDetail2.has_value() && !mainApi -> gameobjExists(objectToDetail2.value())){
        objectToDetail2 = std::nullopt;
    }
    
    std::string name = "[unknown]";
    if (objectToDetail2.has_value()){
        name = mainApi -> getGameObjNameForId(objectToDetail2.value()).value();
    }
    std::cout << "object to detail: " << print(objectToDetail2) << "     " << name << std::endl;
    return objectToDetail2;
}

void renderObjectDetailsWithState(bool includePanel){
    auto objectToDetail = getGameObjectForDetail();
    std::cout << "objectdetails: " << print(objectToDetail) << std::endl;
    if (objectToDetail.has_value()){
        auto exists = mainApi -> gameobjExists(objectToDetail.value());
        if (exists){
            renderObjectDetails(objectToDetail.value(), includePanel);
        }else{
            renderObjectDetails(0, includePanel);
        }
    }else{
        renderObjectDetails(0, includePanel);
    }
}
void renderScenegraphWithState(bool includePanel){
   auto selectedId = renderScenegraph("scenegraph", includePanel, getGameObjectForDetail());
   if (selectedId.has_value()){
     std::set<objid> selectedIds { selectedId.value() };
     mainApi -> setSelected(selectedIds);
   }
}

void renderWidget2(WidgetMenuItem2& item, bool includePanel){
    auto sceneId = currSceneId();
    auto objectToDetail = getGameObjectForDetail();
    item.render(includePanel, objectToDetail, sceneId);
}

float sidebar(const char* title, std::vector<WidgetMenuItem2>& widgets2){
    ImGui::Begin(title, nullptr);
        ImVec2 size = ImGui::GetContentRegionAvail();

        for (int i = 0; i < widgets2.size(); i++){
            auto& widget = widgets2.at(i);
            ImGui::BeginChild(std::to_string(i).c_str(), ImVec2(size.x, size.y * 0.5), true);
                renderWidget2(widget, false);
            ImGui::EndChild();
        }

    float width = ImGui::GetWindowWidth();

    ImGui::End();
    return width;
}

void renderUi(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    renderNavbar();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    if (menuViewState == MENUVIEW_NONE){

    }else if (menuViewState == MENUVIEW_EDITOR){
        std::vector<WidgetMenuItem2> leftWidgets2 {
            WidgetMenuItem2 {
                .name = "Scenegraph",
                .render = [](bool includePanel, std::optional<objid>, std::optional<objid>) -> void {
                    renderScenegraphWithState(includePanel);
                },
            },
        };

        std::vector<WidgetMenuItem2> rightWidgets2 {
            WidgetMenuItem2 {
                .name = "Object Details",
                .render = [](bool includePanel, std::optional<objid>, std::optional<objid>) -> void {
                    renderObjectDetailsWithState(includePanel);
                },
            },
            WidgetMenuItem2 {
                .name = "Object Type",
                .render = [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid>) -> void {
                    renderObjPanel(includePanel, objectToDetail);
                },
            },

        };

        float paddedOffset = viewport -> WorkSize.x * 0.005;

        float leftPaneWidth = viewport -> WorkSize.x * 0.125f;
    	ImGui::SetNextWindowPos(ImVec2(viewport -> WorkPos.x - paddedOffset, viewport -> WorkPos.y));
    	ImGui::SetNextWindowSize(ImVec2(leftPaneWidth , viewport -> WorkSize.y), ImGuiCond_FirstUseEver);
    	sidebar("Scenegraph", leftWidgets2);

        static float rightPaneWidth = viewport -> WorkSize.x * 0.125f;
        ImGui::SetNextWindowPos(ImVec2(viewport -> WorkSize.x - rightPaneWidth + paddedOffset, viewport -> WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(rightPaneWidth, viewport -> WorkSize.y), ImGuiCond_FirstUseEver);
        rightPaneWidth = sidebar("GameObject Details", rightWidgets2);
    }
    
    for (auto &dynamicWidget : dynamicWidgets){
        if (dynamicWidgetEnabled.count(dynamicWidget.id) > 0){
            renderWidget2(dynamicWidget, true);
        }
    }

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#endif
