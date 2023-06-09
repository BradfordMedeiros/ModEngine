#include "./perf-visualize.h"

extern CustomApiBindings* mainApi;

// probably should consider just putting this somewhere in the core engine, but here is fine / more disposable for now

struct PerfVisualize {
	
};

const double totalWidthSeconds = 20;
const double maxHeightSeconds = 0.1;  // ~ 1 fps 
const std::vector<glm::vec4> colorPatterns = { 
	glm::vec4(1.f, 0.f, 0.f, 1.f), 
	glm::vec4(0.f, 1.f, 0.f, 1.f), 
	glm::vec4(0.f, 0.f, 1.f, 1.f) 
};

CScriptBinding cscriptCreatePerfVisualizeBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/perfviz", api);

  //binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  //	PerfVisualize* perfVisualize = new PerfVisualize;
  //  return perfVisualize;
  //};
  //binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  //	PerfVisualize* perfVisualize = static_cast<PerfVisualize*>(data);
  //	delete perfVisualize;
  //};

  binding.onFrame = [](int32_t id, void* data) -> void {
  	//PerfVisualize* perfVisualize = static_cast<PerfVisualize*>(data);
  	return; /*
  	auto sample = mainApi -> getFrameInfo();

  	float adjustedTime = fmod(sample.currentTime, totalWidthSeconds);
  	auto width = sample.totalFrameTime / totalWidthSeconds;

  	float stackedHeight = 0.f;
  	for (int i = 0; i < sample.time.size(); i++){
  		double time = sample.time.at(i);
  		double x = adjustedTime / totalWidthSeconds;  
  		double xRight = (adjustedTime + time) / totalWidthSeconds;	
  		double y = stackedHeight;
  		double yTop = stackedHeight + (time / maxHeightSeconds);
   		stackedHeight = yTop;

   		double width = (xRight - x);
   		double halfWidth = width * 0.5f;
   		double height = (yTop - y);
   		double halfHeight = height * 0.5f;

   		mainApi -> drawRect(
				-1.f + 2 * (x + halfWidth), 
				-1.f + 2 * (y + halfHeight), 
				2 * width,  
				2 * height, 
				true,
				colorPatterns.at(i % 3), 
				std::nullopt, 
				true, 
				std::nullopt,
				std::nullopt
			);
  	}*/

  };



  return binding;
}
