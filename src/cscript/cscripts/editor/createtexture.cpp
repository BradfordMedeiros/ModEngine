#include "./createtexture.h"

const char* textureName = "graphs-testplot";
const float xRangeSecs = 10;
const float yRange = 400;
const std::vector<float> gridLines = { 0.5f, 0.25f, 0.f, -0.5f };
const char* statname = "fps";
const float minDrawingTime = 0.05f;


extern CustomApiBindings* mainApi;

void createObj(objid sceneId){
  GameobjAttributes attr {
    .stringAttributes = {
    	{ "mesh",  "./res/models/box/spriteplane.dae" },
    },
    .numAttributes = {},
    .vecAttr = { 
    	.vec3 = {
    		{ "position", glm::vec3(1.f, 1.f, 0.f) },
    	}, 
    	.vec4 = {} 
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto objectId = mainApi -> makeObjectAttr(sceneId, "performance-test-obj", attr, submodelAttributes); 
  modassert(objectId.has_value(), "cannot create performance-test-obj, probably already exists");

  auto planeObj = mainApi -> getGameObjectByName("performance-test-obj/Plane", sceneId, true);
  GameobjAttributes newAttr {
    .stringAttributes = {{ "texture", textureName }},
    .numAttributes = {  },
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  mainApi -> setGameObjectAttr(planeObj.value(), newAttr);
}

struct CreateTexture {
	float pointX;
	float pointY;
	float lastTime;
	std::optional<unsigned int> textureId;
	objid ownerId;
};

float convertX(float value){ // map between -1 and 1 (from percentage)
	float remainder = fmod(value, xRangeSecs);
	float ratio = remainder / xRangeSecs;
	return (2 * ratio) - 1.f;  

}
float convertY(float value){  // map between -1 and 1 (from percentage)
	float ratio = value / yRange;
	return std::min(1.f, std::max(-1.f, (2 * ratio) - 1));
}
float unconvertY(float value){
	return yRange * 0.5f * (value - 1);
}

void drawGridLine(CreateTexture& createTexture, float yloc){
  mainApi -> drawLine(glm::vec3(-1.f, yloc, 0.f), glm::vec3(1.f, yloc, 0.f), false, createTexture.ownerId, std::nullopt, createTexture.textureId.value(), std::nullopt);
}


float convert(float res, float value){
	return 0.5f * res * (value + 1);
}

void labelGridLine(CreateTexture& createTexture, float yloc){
  mainApi -> drawText(std::to_string(static_cast<int>(round(unconvertY(yloc)))), 10,  convert(1000, yloc) + 20.f, 6, false, std::nullopt, createTexture.textureId.value(), false, std::nullopt, std::nullopt);
  mainApi -> drawText(std::to_string(static_cast<int>(round(unconvertY(yloc)))), 950, convert(1000, yloc) + 20.f, 6, false, std::nullopt, createTexture.textureId.value(), false, std::nullopt, std::nullopt);
}

void drawGrid(CreateTexture& createTexture){
	for (auto yLocationOfLine : gridLines){
		drawGridLine(createTexture, yLocationOfLine);
		labelGridLine(createTexture, yLocationOfLine);
	}
}

void addPermaData(CreateTexture& createTexture){
	drawGrid(createTexture);
  mainApi -> drawText("Performance - Graph of Framerate",  20, 30, 4, false, std::nullopt, createTexture.textureId.value(), false, std::nullopt, std::nullopt);
  mainApi -> drawText(statname, 950, 30, 4, false, std::nullopt, createTexture.textureId.value(), false, std::nullopt, std::nullopt);
  mainApi -> drawLine(glm::vec3(-1.f, 0.9f, 0.f), glm::vec3(1.f, 0.9f, 0.f), false, createTexture.ownerId, std::nullopt, createTexture.textureId.value(), std::nullopt);
  mainApi -> drawLine(glm::vec3(-1.f, 1.f, 0.f), glm::vec3(1.f, 1.f, 0.f), false, createTexture.ownerId, std::nullopt, createTexture.textureId.value(), std::nullopt);
}

void refreshGraph(CreateTexture& createTexture){
	mainApi -> clearTexture(createTexture.textureId.value(), std::nullopt, std::nullopt, std::nullopt);
	addPermaData(createTexture);
}

void plotPoint(CreateTexture& createTexture, float x, float y){
	auto isFirstPoint = x < createTexture.pointX;
	if (!isFirstPoint){  // (and pointX pointY (not firstPoint))
	  mainApi -> drawLine(
	  	glm::vec3(createTexture.pointX, createTexture.pointY, 100.f), 
	  	glm::vec3(x, y, 100.f), 
	  	false, 
	  	createTexture.ownerId, 
	  	std::nullopt, 
	  	createTexture.textureId.value(), 
	  	std::nullopt
	  );
	}else {
		refreshGraph(createTexture);
	}
	createTexture.pointX = x;
	createTexture.pointY = y;
}


CScriptBinding cscriptCreateTextureBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/createtexture", api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    CreateTexture* createTexture = new CreateTexture;
    createTexture -> pointX = 0.f;
    createTexture -> pointY = 0.f;
    createTexture -> lastTime = mainApi -> timeSeconds(true);
    createTexture -> textureId = mainApi -> createTexture(textureName, 1000, 1000, id);
    createTexture -> ownerId = id;
    addPermaData(*createTexture);
    return createTexture;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    CreateTexture* createTexture = static_cast<CreateTexture*>(data);
    delete createTexture;
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    CreateTexture* createTexture = static_cast<CreateTexture*>(data);
    auto now = mainApi -> timeSeconds(true);
    auto diff = now - createTexture -> lastTime;
    if (diff > minDrawingTime){
    	createTexture -> lastTime = now;
    	auto fpsAttr = mainApi -> runStats(statname);
    	auto fps = std::get_if<float>(&fpsAttr);
    	modassert(fps, "fps must be float type");
    	auto coordX = convertX(now);
    	auto coordY = convertY(*fps);

    	modlog("performance", "(time = " + print(now) + "[" + print(coordX) + "]" + ", fps = " + print(fps) + "[" + print(coordY) + "]");
    	plotPoint(*createTexture, coordX, coordY);
    }
  };

  binding.onKeyCharCallback = [](int32_t id, void* data, unsigned int key) -> void {
  	CreateTexture* createTexture = static_cast<CreateTexture*>(data);
  	if (key == '.'){
  		refreshGraph(*createTexture);
  	}else if (key == ','){
  		createObj(mainApi -> listSceneId(id));
  	}
  };


  return binding;
}
