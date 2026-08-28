#include "./gui.h"

#ifndef USE_IMGUI

void initUi(){}
void renderUi(){}
void registerWidget(std::string name, std::optional<std::string> list, std::function<void(bool includePanel,  std::optional<objid> objectToDetail, std::optional<objid> sceneId)> render){}
void registerAction(std::string name, std::string list, std::function<void()> fn){}
void registerView(std::string name, bool hide, std::vector<std::string> leftWidgets, std::vector<std::string> rightWidgets){}

#else 

extern CustomApiBindings* mainApi;
extern GLFWwindow* window;

struct RegisteredAction {
    std::string name;
    std::function<void()> fn;
};

struct RegisteredActions {
    std::string list;
    std::vector<RegisteredAction> actions;
};

std::vector<RegisteredActions> registeredActionLists;
std::vector<WidgetMenuItem2> dynamicWidgets {};
std::vector<std::string> widgetLists;
std::set<int> dynamicWidgetEnabled;

std::vector<ViewMenuItem> dynamicViews;
std::optional<int> currentDynamicView;


#include <filesystem>

std::optional<std::string> FileExplorer(std::string directory){

    for (auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_directory())
        {
            if (ImGui::TreeNode(entry.path().filename().string().c_str()))
            {
                auto selectedFile = FileExplorer(entry.path());
                ImGui::TreePop();
                if (selectedFile.has_value()){
                    return selectedFile.value();
                }
            }
        }
        else{
            if(ImGui::Selectable(entry.path().filename().string().c_str())){
                return entry.path();
            }
        }
    }
    return std::nullopt;

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
        renderLightPanel(includePanel, objectToDetail);
    });  

    registerWidget("Object - Mesh", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderMeshPanel(includePanel, objectToDetail);
    });  

    registerWidget("Particle", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderParticlePanel(includePanel, objectToDetail, sceneId);
    });  

    registerWidget("Object - Sound", "default", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderSoundPanel(includePanel, objectToDetail);
    });  


    registerWidget("Scenegraph", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderScenegraphWithState(includePanel);
    });  

   
    registerWidget("Object Details", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderObjectDetailsWithState(includePanel);
    });  
             

    registerWidget("Object Type", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderObjPanel(includePanel, objectToDetail, sceneId);
    });  


    registerView("Editor", false, { "Scenegraph" }, { "Object Details", "Object Type" }, DIVIDED_LAYOUT);
}

void renderConsole(){
ImGui::Begin("Console");

ImGui::BeginChild("Log", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);

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


void registerWidget(std::string name, std::optional<std::string> list, std::function<void(bool includePanel,  std::optional<objid> objectToDetail, std::optional<objid> sceneId)> render){
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
        if (!list.has_value() ||  widgetList == list.value()){
            alreadyHasList = true;
            break;
        }
    }
    if (!alreadyHasList){
        widgetLists.push_back(list.value());
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


void registerView(std::string name, bool hide, std::vector<std::string> leftWidgetStrs, std::vector<std::string> rightWidgetStrs, ViewType viewType){
    int id = getSymbol(name);

    std::vector<WidgetMenuItem2> leftWidgets;
    std::vector<WidgetMenuItem2> rightWidgets;
    for (auto& leftWidgetStr : leftWidgetStrs){
        auto widget = widgetByName(leftWidgetStr);
        modassert(widget.has_value(), "register view missing widget");
        leftWidgets.push_back(*widget.value());
    }
    for (auto& rightWidgetStr : rightWidgetStrs){
        auto widget = widgetByName(rightWidgetStr);
        modassert(widget.has_value(), "register view missing widget");
        rightWidgets.push_back(*widget.value());
    }

    dynamicViews.push_back(ViewMenuItem {
        .id = id,
        .hide = hide,
        .name = name,
        .type = viewType,
        .leftWidgets = leftWidgets,
        .rightWidgets = rightWidgets,
    });
}

std::optional<ViewMenuItem*> viewByName(int symbol){
    for (auto& view : dynamicViews){
        if (view.id == symbol){
            return &view;
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
    if (ImGui::BeginMainMenuBar()){
        // File menu
        if (ImGui::BeginMenu("File")){
            if (ImGui::MenuItem("Exit")){
                exit(0);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")){
            bool showNone = !currentDynamicView.has_value();
            if(ImGui::MenuItem("None", nullptr, showNone)){
                currentDynamicView = std::nullopt;
            }    
            for (auto& dynamicView : dynamicViews){
                if (dynamicView.hide){
                    continue;
                }
                bool showEditor = currentDynamicView.has_value() && currentDynamicView.value() == dynamicView.id;
                if(ImGui::MenuItem(dynamicView.name.c_str(), nullptr, showEditor)){
                    currentDynamicView = dynamicView.id;
                }
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
                    if (!widgetMenuItem.list.has_value() || widgetMenuItem.list.value() != widgetList){
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

float sidebar2(const char* title, std::vector<WidgetMenuItem2>& widgets2){
    ImGui::Begin(title, nullptr,  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
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


void renderDividedLayout(ViewMenuItem& view){
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float paddedOffset = viewport -> WorkSize.x * 0.005;
    float leftPaneWidth = viewport -> WorkSize.x * 0.125f;
    ImGui::SetNextWindowPos(ImVec2(viewport -> WorkPos.x - paddedOffset, viewport -> WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(leftPaneWidth , viewport -> WorkSize.y), ImGuiCond_FirstUseEver);
    sidebar(view.leftWidgets.at(0).name.c_str(), view.leftWidgets);

    static float rightPaneWidth = viewport -> WorkSize.x * 0.125f;
    ImGui::SetNextWindowPos(ImVec2(viewport -> WorkSize.x - rightPaneWidth + paddedOffset, viewport -> WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(rightPaneWidth, viewport -> WorkSize.y), ImGuiCond_FirstUseEver);
    rightPaneWidth = sidebar(view.rightWidgets.at(0).name.c_str(), view.rightWidgets);
}

void renderSplitLayout(ViewMenuItem& view){
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float paddedOffset = viewport -> WorkSize.x * 0.00;
    float verticalOffset = 0.f;

    float leftPanelWidth = viewport -> WorkSize.x * 0.125f;
    float leftPanelX = viewport -> WorkPos.x - paddedOffset;
    float leftPanelXRight = leftPanelX + leftPanelWidth;

    ImGui::SetNextWindowPos(ImVec2(leftPanelX, viewport -> WorkPos.y + verticalOffset));
    ImGui::SetNextWindowSize(ImVec2(leftPanelWidth , viewport -> WorkSize.y), ImGuiCond_Always);
    sidebar2(view.leftWidgets.at(0).name.c_str(), view.leftWidgets);

    float rightPaneWidth = viewport -> WorkSize.x * 0.875f;
    ImGui::SetNextWindowPos(ImVec2(leftPanelXRight, viewport -> WorkPos.y + verticalOffset));
    ImGui::SetNextWindowSize(ImVec2(rightPaneWidth, viewport -> WorkSize.y), ImGuiCond_Always);
    rightPaneWidth = sidebar2(view.rightWidgets.at(0).name.c_str(), view.rightWidgets);  
}


struct BufferedTextImGui {
    std::string text;
};
static std::vector<BufferedTextImGui> bufferedTextImGui;



void renderLayout(ViewMenuItem& dynamicView){
    if (dynamicView.type == SPLIT_LAYOUT){
        renderSplitLayout(dynamicView);
    }else if (dynamicView.type == DIVIDED_LAYOUT){
        renderDividedLayout(dynamicView);
    }
}

std::optional<std::function<void()>> additionalUserGui;

void setGuiFn(std::optional<std::function<void()>> fn){
    additionalUserGui = fn;
}

void renderUi(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    renderNavbar();

    if (currentDynamicView.has_value()){
        for (auto& dynamicView : dynamicViews){
            if (currentDynamicView.value() == dynamicView.id){
                renderLayout(dynamicView);
                break;
            }
        }
    }

    for (auto &dynamicWidget : dynamicWidgets){
        if (dynamicWidgetEnabled.count(dynamicWidget.id) > 0){
            renderWidget2(dynamicWidget, true);
        }
    }

  
    //std::cout << "imgui text: " << bufferedTextImGui.size() << std::endl;
    //for (auto& bufferedText : bufferedTextImGui){
    //    ImGui::Text(bufferedText.text.c_str());
    //}

    if (additionalUserGui.has_value()){
        additionalUserGui.value()();
    }

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}


void drawImGuiText(std::string text){
    bufferedTextImGui.push_back(BufferedTextImGui {
        .text = text,
    });
}

void clearImGuiData(){
    bufferedTextImGui = {};
}

#endif
