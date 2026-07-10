#include "./gui.h"

#ifndef USE_IMGUI

void initUi(){}
void renderUi(){}

#else 

extern GLFWwindow* window;

void initUi(){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void renderScenegraph(){
  ImGui::Text("Hello World");
  ImGui::Button("Click me");

  static float color2[4] = {1.0f, 0.2f, 0.3f, 1.0f};
  ImGui::ColorPicker4("Light Color", color2);

  static int count = 10;
  ImGui::SliderInt("Count", &count, 0, 100);

  static float value1 = 0.5f;
  ImGui::SliderFloat("Value", &value1, 0.0f, 1.0f);

  static float position[3] = {0.0f, 1.0f, 2.0f};
  ImGui::SliderFloat3("Position", position, -100.0f, 100.0f);

  static bool showOption = false;
  static std::string testname = "test name";

  ImGui::Checkbox("option name", &showOption);
  ImGui::InputText("Object Name", &testname);


  if (ImGui::BeginPopupContextItem()){
      if (ImGui::MenuItem("Add Child"))
      {
          // create node
      }
  
      if (ImGui::MenuItem("Delete"))
      {
          // remove node
      }
  
      ImGui::EndPopup();
  }
  
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

void renderNavbar(){
    if (ImGui::BeginMainMenuBar())
    {
        // File menu
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
                // Create new scene
            }

            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                // Open file dialog
            }

            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                // Save scene
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                // Quit application
            }

            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
                // Undo
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Y"))
            {
                // Redo
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete", "Del"))
            {
                // Delete selected object
            }

            ImGui::EndMenu();
        }

        // View menu
        if (ImGui::BeginMenu("View"))
        {
            static bool showHierarchy = true;
            static bool showInspector = true;

            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &showInspector);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void renderObjectDetails(){
  ImGui::Begin("Object Details");
  ImGui::Text("Hello World");
  ImGui::Button("Click me");

  static bool showOption = true;
  static std::string testname = "hello world";
  ImGui::Checkbox("option name", &showOption);
  ImGui::InputText("Object Name", &testname);

  if (ImGui::BeginPopupContextItem()){
      if (ImGui::MenuItem("Add Child"))
      {
          // create node
      }
  
      if (ImGui::MenuItem("Delete"))
      {
          // remove node
      }
  
      ImGui::EndPopup();
  }
  
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

void renderActiveScene(){
  ImGui::Begin("Active Scene");

  ImGui::Text("Active Id = [19323939]");
  ImGui::Button("Save Scene");
  ImGui::Button("Reset Scene");

  ImGui::End();
}

void renderUi(){

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

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

  renderNavbar();



  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(
      ImGui::GetDrawData()
  );

}

#endif
