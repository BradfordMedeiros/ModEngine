#include "./widgets.h"

extern CustomApiBindings* mainApi;

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

void renderObjectCount(){
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


	ImGui::Begin("Object Count");

	for (auto& object : objects){
		auto statSymbol = mainApi -> stat(object.statName);
		auto statValue = mainApi -> statValue(statSymbol);
		auto objectCount = std::get_if<int>(&statValue);
		modassert(objectCount, "object count is NULL");

		ImGui::Text(object.field.c_str());
		ImGui::SameLine();
		ImGui::Text(std::to_string(*objectCount).c_str());
	}

	ImGui::End();
}

void renderActiveScene(){
  ImGui::Begin("Active Scene");

  ImGui::Text("Active Id = [19323939]");
  ImGui::Button("Save Scene");
  ImGui::Button("Reset Scene");

  ImGui::End();
}