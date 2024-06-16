#include "./perf-visualize.h"

extern CustomApiBindings* mainApi;

const double totalWidthSeconds = 20;
const double maxHeightSeconds = 0.020;  // ~ 50 fps 
const std::vector<glm::vec4> colorPatterns = { 
	glm::vec4(1.f, 0.f, 0.f, 1.f), 
	glm::vec4(0.f, 1.f, 0.f, 1.f), 
	glm::vec4(0.f, 0.f, 1.f, 1.f), 
	glm::vec4(1.f, 0.f, 1.f, 1.f), 
	glm::vec4(0.f, 1.f, 1.f, 1.f), 

};

void printFrameInfo(FrameInfo& frameInfo){
  std::cout << "current time: " << frameInfo.currentTime << std::endl;
  std::cout << "totalFrameTime: " << frameInfo.totalFrameTime << std::endl;
  std::cout << "time: ";
  for (auto time : frameInfo.time){
    std::cout << time << " ";
  }
  std::cout << std::endl;
  std::cout << "labels: ";
  for (auto label : frameInfo.labels){
    std::cout << label << " ";
  }
  std::cout << std::endl;
}


CScriptBinding cscriptCreatePerfVisualizeBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/perfviz", api);

  binding.onKeyCharCallback = [](int32_t id, void* data, unsigned int key) -> void {
  	auto frameInfo = mainApi -> getFrameInfo();
  	printFrameInfo(frameInfo);
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
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

   		auto color = colorPatterns.at(i % colorPatterns.size());
   		mainApi -> drawRect(
				-1.f + 2 * (x + halfWidth), 
				-1.f + 2 * (y + halfHeight), 
				2 * width,  
				2 * height, 
				true,
				color, 
				std::nullopt, 
				true, 
				std::nullopt,
				std::nullopt
			);
    	mainApi -> drawText(sample.labels.at(i), 0, 0.2 + i * 0.1, 8, false, color, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  	}

  };



  return binding;
}
