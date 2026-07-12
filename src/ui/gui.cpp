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

enum ImMenuView { MENUVIEW_NONE, MENUVIEW_EDITOR, MENUVIEW_SCENEGRAPH, MENUVIEW_DEBUG, MENUVIEW_BALL, MENUVIEW_MODEL };
ImMenuView menuViewState = MENUVIEW_NONE;

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
            bool showDebug = menuViewState == MENUVIEW_DEBUG;
            bool showBall = menuViewState == MENUVIEW_BALL;

            if(ImGui::MenuItem("None", nullptr, showNone)){
            	menuViewState = MENUVIEW_NONE;
            }
            if(ImGui::MenuItem("Editor", nullptr, showEditor)){
            	menuViewState = MENUVIEW_EDITOR;
            }
            if(ImGui::MenuItem("Scenegraph", nullptr, showScene)){
            	menuViewState = MENUVIEW_SCENEGRAPH;
            }
            if (ImGui::MenuItem("Debug", nullptr, showDebug)){
            	menuViewState = MENUVIEW_DEBUG;
            }

  			ImGui::Dummy(ImVec2(0, 10));
            if (ImGui::MenuItem("Stats", nullptr, showDebug)){

            }

  			ImGui::Dummy(ImVec2(0, 10));
        	if (ImGui::MenuItem("Ball Options", nullptr, showBall)){
            	menuViewState = MENUVIEW_BALL;
            }




            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Widget")){

            if (ImGui::MenuItem("Object counts")){

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

void renderObjectDetails(objid id){
  auto name = mainApi -> getGameObjNameForId(id).value();

  ImGui::Begin("Object Details");

  std::string objectName = std::string("Name: ") + name;
  ImGui::Text(objectName.c_str());

  static std::string testname = "hello world";
  ImGui::InputText("Rename Object", &testname);
  ImGui::Button("Rename");
  ImGui::Dummy(ImVec2(0, 10));

  static bool showOption = true;
  ImGui::Checkbox("Enable Physics", &showOption);
  ImGui::Checkbox("Dynamic", &showOption);
  ImGui::Checkbox("Collision", &showOption);

  ImGui::Text("Physics Shape");

  ImGui::Checkbox("Box", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Sphere", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Capsule", &showOption);
  
  ImGui::Checkbox("Cylinder", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Hull", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Auto", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Exact", &showOption);

  static float position[3] = {0.0f, 1.0f, 2.0f};
  ImGui::Dummy(ImVec2(0, 10));

  ImGui::Text("Position");
  ImGui::PushItemWidth(70);
  ImGui::DragFloat("X", &position[0], 0.1f);
  ImGui::SameLine();
  ImGui::DragFloat("Y", &position[1], 0.1f);
  ImGui::SameLine();
  ImGui::DragFloat("Z", &position[2], 0.1f);
  ImGui::PopItemWidth();

  ImGui::Text("Scale");
  ImGui::PushItemWidth(70);
  ImGui::DragFloat("X", &position[0], 0.1f);
  ImGui::SameLine();
  ImGui::DragFloat("Y", &position[1], 0.1f);
  ImGui::SameLine();
  ImGui::DragFloat("Z", &position[2], 0.1f);
  ImGui::PopItemWidth();

  auto objectDetailsSize = ImGui::GetWindowSize();
  std::cout << "object details size: " << objectDetailsSize.x << std::endl;

  ImGui::End();
}

void renderCameraPanel(){
  ImGui::Begin("Cameras");

  ImGui::Button("Create Camera");

  static bool doThing = false;
  ImGui::Checkbox("Depth of Field", &doThing);
  
 float speed = 5.0f;

  ImGui::SliderFloat("Min Blur", &speed, 0.0f, 10.0f);
  ImGui::SliderFloat("Max Blur", &speed, 0.0f, 10.0f);
  ImGui::SliderFloat("Blur Amount", &speed, 0.0f, 10.0f);

  ImGui::End();
}

void renderLightPanel(){
  ImGui::Begin("Light");

  ImGui::Button("Create Light");

  static bool showOption = false;
  ImGui::Text("Type");

  ImGui::Checkbox("Point", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Spotlight", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Directional", &showOption);

  ImGui::End();
}

void renderDebug(){
  ImGui::Begin("Debug Panel");
  static bool doThing = false;
  ImGui::Checkbox("Show Debug", &doThing);
  ImGui::Checkbox("Show Cameras", &doThing);
  ImGui::Checkbox("Show Lights", &doThing);
  ImGui::Checkbox("Show Sounds", &doThing);
  ImGui::Checkbox("Show Emitters", &doThing);

  ImGui::End();
}

void renderPlayMode(){
  ImGui::Begin("Play Mode");
  static bool doThing = false;
  ImGui::Checkbox("Play", &doThing);
  ImGui::Checkbox("Free Cam", &doThing);
  ImGui::Checkbox("Editor", &doThing);
  ImGui::End();
}

void renderBallGameplay(){
  ImGui::Begin("Ball Gameplay");
  static bool doThing = false;

  static float speed = 0.f;
  ImGui::DragFloat("jump", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("magnitude", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("torque", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("jump-magnitude", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("mass", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("friction", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("restitution", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("gravity", &speed, 0.0f, 10.0f);

  ImGui::End();
}


void renderActiveScene(){
  ImGui::Begin("Active Scene");

  ImGui::Text("Active Id = [19323939]");
  ImGui::Button("Save Scene");
  ImGui::Button("Reset Scene");

  ImGui::End();
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


void RenderEditor()
{
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

void leftSidebar(){
    ImGui::Begin(
        "Editor",
        nullptr
    );

    ImVec2 size = ImGui::GetContentRegionAvail();
    /*    ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize*/

    ImGui::BeginChild(
        "Assets",
        ImVec2(size.x, size.y * 0.5),
        true
    );

    FileExplorer(".");

    ImGui::EndChild();




    ImGui::BeginChild(
        "Viewport 1",
        ImVec2(size.x , size.y * 0.5),
        true
    );

    ImGui::Text("Game viewport 1");

    ImGui::EndChild();

    ImGui::BeginChild(
        "Viewport 2",
        ImVec2(size.x , size.y * 0.5),
        true
    );

    ImGui::Text("Game viewport 2");

    ImGui::EndChild();

    ImGui::BeginChild(
        "Viewport 3",
        ImVec2(size.x , size.y * 0.5),
        true
    );

    ImGui::Text("Game viewport 3");

    ImGui::EndChild();


    ImGui::BeginChild(
        "Viewport 4",
        ImVec2(size.x , size.y * 0.5),
        true
    );
static char text[4096] = "sdfasljdhalsjhdfjklhs";

ImGui::InputTextMultiline(
    "##editor",
    text,
    sizeof(text),
    ImVec2(500, 300)
);

    ImGui::EndChild();


    ImGui::End();

      ImGui::Begin(
        "Editor Text",
        nullptr
    );

ImGui::InputTextMultiline(
    "##editortext",
    text,
    sizeof(text),
    ImVec2(500, 300)
);

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
    	ImGui::SetNextWindowPos(viewport->WorkPos);
    	auto size = viewport -> WorkSize;
    	ImGui::SetNextWindowSize(ImVec2(size.x * 0.2 , size.y));
    	leftSidebar(); 	
    }else if (menuViewState == MENUVIEW_SCENEGRAPH){
        static std::optional<objid> objectToDetail;

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

        if (objectToDetail.has_value()){
            auto exists = mainApi -> gameobjExists(objectToDetail.value());
            if (!exists){
                objectToDetail = std::nullopt;
            }else{
                renderObjectDetails(objectToDetail.value());
            }
        }

    }else if (menuViewState == MENUVIEW_DEBUG){
    	renderDebug();
    	renderPlayMode();
    }else if (menuViewState == MENUVIEW_BALL){
    	renderBallGameplay();
    }





    /*ImGui::BeginChild(
        "Viewport 2",
        ImVec2(0, size.y),
        true
    );

    ImGui::Text("Game viewport 2");

    ImGui::EndChild();*/



  //ImGui::Begin("Scenegraph");
  //renderScenegraph();
  //ImGui::End();
/*
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

 // ImGui::Begin("Editor",  nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  renderNavbar();
ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->WorkPos);
ImGui::SetNextWindowSize(viewport->WorkSize);


   ImVec2 size = ImGui::GetContentRegionAvail();


  ImGui::BeginChild(
    "Viewport",
    ImVec2(300, size.y - 10),
    true


  ImGui::Begin("Assets", nullptr,  ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoCollapse);
  FileExplorer(".");
  ImGui::End();

  ImGui::EndChild();

  RenderEditor();
	//DrawDockSpace();      // One dockspace host

	//  ImGui::End();
*/
/*
  renderObjectDetails();
  renderDebug();
  renderActiveScene();
  
  ImGui::SetNextWindowSizeConstraints(
      ImVec2(300, 0),        // minimum size
      ImVec2(FLT_MAX, FLT_MAX) // maximum size
  );

  ImGui::Begin("Editor",  nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::CollapsingHeader("Scenegraph")){
  	  renderScenegraph();
  }
  if (ImGui::CollapsingHeader("Scenegraph2")){
  	  renderScenegraph();
  }
  if (ImGui::CollapsingHeader("Scenegraph3")){
  	  renderScenegraph();
  }
  if (ImGui::CollapsingHeader("Scenegraph4")){
  	  renderScenegraph();
  }
  ImGui::End();



  renderConsole();
*/




  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(
      ImGui::GetDrawData()
  );

}

#endif
