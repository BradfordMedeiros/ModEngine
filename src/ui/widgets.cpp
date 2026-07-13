#include "./widgets.h"

extern CustomApiBindings* mainApi;

void renderDebug(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Debug Panel");
	}
  static bool doThing = false;
  ImGui::Checkbox("Show Debug", &doThing);
  ImGui::Checkbox("Show Cameras", &doThing);
  ImGui::Checkbox("Show Lights", &doThing);
  ImGui::Checkbox("Show Sounds", &doThing);
  ImGui::Checkbox("Show Emitters", &doThing);

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

void renderActiveScene(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Active Scene");
	}

  ImGui::Text("Active Id = [19323939]");
  ImGui::Button("Save Scene");
  ImGui::Button("Reset Scene");

  if (includePanel){
	  ImGui::End();
  }
}

void renderCameraPanel(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Cameras");
	}

  ImGui::Button("Create Camera");

  static bool doThing = false;
  ImGui::Checkbox("Depth of Field", &doThing);
  
 float speed = 5.0f;

  ImGui::SliderFloat("Min Blur", &speed, 0.0f, 10.0f);
  ImGui::SliderFloat("Max Blur", &speed, 0.0f, 10.0f);
  ImGui::SliderFloat("Blur Amount", &speed, 0.0f, 10.0f);

  if (includePanel){
	  ImGui::End();
  }
}

void renderLightPanel(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Light");
	}

  ImGui::Button("Create Light");

  static bool showOption = false;
  ImGui::Text("Type");

  ImGui::Checkbox("Point", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Spotlight", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Directional", &showOption);

  if (includePanel){
	  ImGui::End();
  }
}

void renderBallGameplay(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Ball Gameplay");
	}
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

  if (includePanel){
	  ImGui::End();
  }
}

void renderObjectDetails(objid id, bool includePanel){
  auto name = mainApi -> getGameObjNameForId(id).value();

  if (includePanel){
      ImGui::Begin("Object Details");
  }

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

  if (includePanel){
      ImGui::End();
  }
}