#include "./gui.h"

#ifndef USE_IMGUI

void initUi(){}
void renderUi(){}

#else 

extern CustomApiBindings* mainApi;
extern GLFWwindow* window;

void initUi(){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
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

enum ImMenuView { MENUVIEW_NONE, MENUVIEW_EDITOR, MENUVIEW_SCENEGRAPH, MENUVIEW_MODEL };

enum ImMenuWidgets  { WIDGET_OBJECTCOUNT, WIDGET_DEBUG, WIDGET_ACTIVE_SCENE, WIDGET_CAMERA, WIDGET_LIGHT, WIDGET_BALL, WIDGET_OBJECT_DETAILS };
ImMenuView menuViewState = MENUVIEW_NONE;
std::set<ImMenuWidgets> widgets {};

struct WidgetMenuItem {
    std::string name;
    ImMenuWidgets widget;
};
std::vector<WidgetMenuItem> widgetMenuItems {
    // Generic Global ones
    WidgetMenuItem {
        .name = "Object Count",
        .widget = WIDGET_OBJECTCOUNT,
    },
    WidgetMenuItem {
        .name = "Debug",
        .widget = WIDGET_DEBUG,
    },
    WidgetMenuItem {
        .name = "ActiveScene",
        .widget = WIDGET_ACTIVE_SCENE,
    },

    // Object Specific
    WidgetMenuItem {
        .name = "Object - Light",
        .widget = WIDGET_LIGHT,
    },
    WidgetMenuItem {
        .name = "Object - Camera",
        .widget = WIDGET_CAMERA,
    },

    // Game Specific 

    WidgetMenuItem {
        .name = "Game - Ball",
        .widget = WIDGET_BALL,
    },

};

void renderNavbar(){
    if (ImGui::BeginMainMenuBar())
    {
        // File menu
        if (ImGui::BeginMenu("File"))
        {

            if (ImGui::MenuItem("Exit"))
            {
                // Quit application
            }

            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Mode"))
        {
            if (ImGui::MenuItem("Play", "play the game"))
            {
                // Undo
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Free", "move camera freely"))
            {
            }
            if (ImGui::MenuItem("Edit", "editor mode"))
            {
            }

 
            ImGui::EndMenu();
        }

        // View menu
        if (ImGui::BeginMenu("View"))
        {
            bool showNone = menuViewState == MENUVIEW_NONE;
            bool showEditor = menuViewState == MENUVIEW_EDITOR;
            bool showScene = menuViewState == MENUVIEW_SCENEGRAPH;

            if(ImGui::MenuItem("None", nullptr, showNone)){
            	menuViewState = MENUVIEW_NONE;
            }
            if(ImGui::MenuItem("Editor", nullptr, showEditor)){
            	menuViewState = MENUVIEW_EDITOR;
            }
            if(ImGui::MenuItem("Scenegraph", nullptr, showScene)){
            	menuViewState = MENUVIEW_SCENEGRAPH;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Widget")){
            if(ImGui::MenuItem("Hide All")){
                widgets = {};
            }
            ImGui::Separator();

            for (auto& widgetMenuItem : widgetMenuItems){
                if (ImGui::MenuItem(widgetMenuItem.name.c_str())){
                    if (widgets.count(widgetMenuItem.widget) > 0){
                        widgets.erase(widgetMenuItem.widget);
                    }else{
                        widgets.insert(widgetMenuItem.widget);
                    }
                }
            }

            ImGui::EndMenu();

        }   


        if (ImGui::BeginMenu("Special")){
           if (ImGui::MenuItem("Gun Editor"))
           {
           }

           if (ImGui::BeginMenu("Arcade"))
           {

               if (ImGui::MenuItem("Invaders"))
               {
                    // open scene1
               }

               if (ImGui::MenuItem("Tennis"))
               {
                    // open scene1
               }

               ImGui::EndMenu();

           }

           ImGui::EndMenu();
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

static std::optional<objid> objectToDetail;
void renderObjectDetailsWithState(bool includePanel){
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
void renderWidget(ImMenuWidgets widget, bool includePanel){
    if (widget == WIDGET_OBJECTCOUNT){
        renderObjectCount(includePanel);
    }
    if (widget == WIDGET_DEBUG){
        renderDebug(includePanel);
    }
    if (widget == WIDGET_ACTIVE_SCENE){
        renderActiveScene(includePanel);
    }
    if (widget == WIDGET_CAMERA){
        renderCameraPanel(includePanel);
    }
    if (widget == WIDGET_LIGHT){
        renderLightPanel(includePanel);
    }
    if (widget == WIDGET_BALL){
        renderBallGameplay(includePanel);
    }
    if (widget == WIDGET_OBJECT_DETAILS){
        renderObjectDetailsWithState(includePanel);
    }
}

void leftSidebar(std::vector<WidgetMenuItem>& widgets){
    ImGui::Begin("Editor", nullptr);
        ImVec2 size = ImGui::GetContentRegionAvail();
        
        for (int i = 0; i < widgets.size(); i++){
            auto& widget = widgets.at(i);
            ImGui::BeginChild(std::to_string(i).c_str(), ImVec2(size.x, size.y * 0.5), true);
                renderWidget(widget.widget, false);
            ImGui::EndChild();
        }

    ImGui::End();
}


void rightSidebar(std::vector<WidgetMenuItem>& widgets){
    ImGui::Begin("Editor2", nullptr);
        ImVec2 size = ImGui::GetContentRegionAvail();
        
        for (int i = 0; i < widgets.size(); i++){
            auto& widget = widgets.at(i);
            ImGui::BeginChild(std::to_string(i).c_str(), ImVec2(size.x , size.y * 0.5), true);
                renderWidget(widget.widget, false);
            ImGui::EndChild();
        }

    ImGui::End();
}




void renderUi(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderNavbar();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    if (menuViewState == MENUVIEW_NONE){

    }else if (menuViewState == MENUVIEW_EDITOR){
        auto size = viewport -> WorkSize;
    	ImGui::SetNextWindowPos(viewport->WorkPos);
    	ImGui::SetNextWindowSize(ImVec2(size.x * 0.2 , size.y));
        std::vector<WidgetMenuItem> widgets {
            WidgetMenuItem {
                .name = "Object Count",
                .widget = WIDGET_OBJECTCOUNT,
            },
            WidgetMenuItem {
                .name = "Object Count",
                .widget = WIDGET_OBJECTCOUNT,
            },
            WidgetMenuItem {
                .name = "Object Count",
                .widget = WIDGET_OBJECT_DETAILS,
            },
        };
    	leftSidebar(widgets);

        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 400, viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(size.x * 0.2 , size.y));
        rightSidebar(widgets);

    }else if (menuViewState == MENUVIEW_SCENEGRAPH){
    	auto size = viewport -> WorkSize;

    	ImGui::SetNextWindowPos(viewport->WorkPos);
    	ImGui::SetNextWindowSize(ImVec2(size.x * 0.2 , size.y), ImGuiCond_FirstUseEver);
    	auto selectedId = renderScenegraph("Scenegraph 1");
        if (selectedId.has_value()){
            objectToDetail = selectedId.value();
        }

		const float rightPaneWidth = viewport -> WorkSize.x * 0.25f;
		ImGui::SetNextWindowPos(ImVec2(viewport -> WorkSize.x - rightPaneWidth,	viewport -> WorkPos.y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(rightPaneWidth, viewport -> WorkSize.y),ImGuiCond_FirstUseEver);

        renderObjectDetailsWithState(true);
    }

    for (auto &widget : widgets){
        renderWidget(widget, true);
    }

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

#endif
