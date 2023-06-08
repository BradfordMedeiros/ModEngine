#include "./perf-visualize.h"

extern CustomApiBindings* mainApi;

// probably should consider just putting this somewhere in the core engine, but here is fine / more disposable for now

struct PerfVisualize {
	
};

struct FrameRenderSample {
	double currentTime;
	double totalFrameTime;
	std::vector<double> time;
};

FrameRenderSample generateSamples(){
	auto time = mainApi -> timeSeconds(true);
	double totalFrameTime = 0.015;  // ~60fps 
	return FrameRenderSample {
		.currentTime = time,
		.totalFrameTime = totalFrameTime,
		.time = { 0.010f, 0.005f },
	};
}

const double totalWidthSeconds = 20;
const double maxHeightSeconds = 0.1;  // ~ 1 fps 
const std::vector<glm::vec4> colorPatterns = { 
	glm::vec4(1.f, 0.f, 0.f, 0.8f), 
	glm::vec4(0.f, 1.f, 0.f, 0.8f), 
	glm::vec4(0.f, 0.f, 1.f, 0.8f) 
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
  	auto sample = generateSamples();
  	auto width = sample.totalFrameTime / totalWidthSeconds;

  	float stackedHeight = 0.f;
  	for (int i = 0; i < sample.time.size(); i++){
  		auto time = sample.time.at(i);
  		double x = sample.currentTime / totalWidthSeconds;
  		double xRight = (sample.currentTime + time) / totalWidthSeconds;
  		double y = stackedHeight;
  		double yTop = stackedHeight + (time / maxHeightSeconds);
   		stackedHeight = yTop;

   		float width = (xRight - x);
   		float halfWidth = width * 0.5f;
   		float height = (yTop - y);
   		float halfHeight = height * 0.5f;

   		mainApi -> drawRect(
				-1.f + x + halfWidth, 
				y + halfHeight, 
				width,  
				height, 
				true,
				colorPatterns.at(i % 3), 
				std::nullopt, 
				true, 
				std::nullopt,
				std::nullopt
			);
  	}

  };



  return binding;
}
