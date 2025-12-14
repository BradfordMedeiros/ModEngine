#include "./effekseer.h"

double timeElapsed();

// notes: 
// This exists, might be useful later, since particles not affected by lighting.  
//effekseerManager -> SetAllColor(handle, Effekseer::Color(255, 255, 255, 255));

Effekseer::ManagerRef effekseerManager;
EffekseerRendererGL::RendererRef effekRenderer;

struct EffectData {
	Effekseer::EffectRef effectRef;
	std::optional<Effekseer::Handle> playingEffect;
	std::optional<glm::vec3> position;
	std::optional<glm::quat> rotation;
	bool loopContinuously;
	std::string effectName;
};
std::unordered_map<objid, EffectData> effekseerData;

/*class MyFileInterface : public Effekseer::DefaultFileReader
{

};

class MyEffectLoader : public Effekseer::EffectLoader
{
    Effekseer::FileInterfaceRef fileInterface_;

public:
    MyEffectLoader(Effekseer::FileInterfaceRef fileInterface)
        : fileInterface_(fileInterface)
    {}

    bool Load(const char16_t* path, void*& data, int32_t& size) override
    {
        std::u16string msg(path);
        std::string utf8(msg.begin(), msg.end());

        std::cout << "[Effekseer/EffectLoader] Load: " << utf8 << std::endl;

        //return Effekseer::EffectLoader::Load(data, size, effect, path);
    }

    void Unload(void* data, int32_t size) override
    {
        std::cout << "[Effekseer/EffectLoader] Unload" << std::endl;
    }
};*/

void initEffekseer(){
  std::cout << "effekseer initialized start" << std::endl;

  Effekseer::SetLogger([](Effekseer::LogType logType, const std::string& value) -> void {
  	std::cout << "effekseer: value is: " << value << std::endl;
  });


  effekseerManager = ::Effekseer::Manager::Create(8000);
  modassert(effekseerManager.Get() != NULL, "could not create effekseer manager");



  effekRenderer = EffekseerRendererGL::Renderer::Create(8000);  // max number of particles to render
  modassert(effekRenderer.Get() != NULL, "could not create effekseer renderer");

  effekRenderer -> SetRestorationOfStatesFlag(true);

  effekseerManager -> SetSpriteRenderer(effekRenderer ->CreateSpriteRenderer());
  effekseerManager -> SetRibbonRenderer(effekRenderer ->CreateRibbonRenderer());
  effekseerManager -> SetRingRenderer(effekRenderer -> CreateRingRenderer());
  effekseerManager -> SetTrackRenderer(effekRenderer -> CreateTrackRenderer());
  effekseerManager -> SetModelRenderer(effekRenderer -> CreateModelRenderer());

  effekseerManager->SetTextureLoader(effekRenderer->CreateTextureLoader());
  effekseerManager->SetModelLoader(effekRenderer->CreateModelLoader());
  effekseerManager->SetMaterialLoader(effekRenderer->CreateMaterialLoader());
  effekseerManager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());
}

bool effectPlaying(EffectData& effect){
	if (!effect.playingEffect.has_value()){
		return false;
	}
	bool effectStillPlaying = effekseerManager->Exists(effect.playingEffect.value());
	return effectStillPlaying;
}




void playEffectsAlways(){
	for (auto& [id, effect] : effekseerData){
		if (effect.loopContinuously && !effectPlaying(effect)){
			glm::vec3 position = effect.position.has_value() ? effect.position.value() : glm::vec3(0.f, 0.f, 0.f); // this should come from
			Effekseer::Handle playingEffect = effekseerManager -> Play(effect.effectRef, position.x, position.y, position.z);
			effect.playingEffect = playingEffect;
			if (effect.rotation.has_value()){
				glm::vec3 euler = glm::eulerAngles(effect.rotation.value());
				effekseerManager->SetRotation(
				    effect.playingEffect.value(),
				    euler.x,
				    euler.y,
				    euler.z
				);
			}
		}
	}	
}

void onEffekSeekerFrame(){
	static bool initialized = false;
	if (!initialized){
		initialized = true;
		initEffekseer();
	}

	playEffectsAlways();

	Effekseer::Manager::UpdateParameter updateParameter;
	effekseerManager -> Update(); // pass in the actual delta time

	//for (int i = 0; i < 10000; i++){
	//	std::cout << "hasdf" << std::endl;
	//}
}

void onEffekSeekerRender(float windowSizeX, float windowSizeY, float fovRadians, glm::vec3 viewPosition, glm::quat viewDirection, float nearPlane, float farPlane){
	std::cout << "effekseer: " << windowSizeX << ", " << windowSizeY << std::endl;

	auto viewerPosition = ::Effekseer::Vector3D(10.0f, 5.0f, 10.0f);
    auto lookAt = ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f);


	{
		viewerPosition = ::Effekseer::Vector3D(viewPosition.x, viewPosition.y, viewPosition.z);

		auto targetPos = viewPosition + (viewDirection * glm::vec3(0.f, 0.f, -1.f));
		lookAt = ::Effekseer::Vector3D(targetPos.x, targetPos.y, targetPos.z);
	}


	Effekseer::Manager::LayerParameter layerParameter;
	layerParameter.ViewerPosition = viewerPosition;
	
	effekseerManager -> SetLayerParameter(0, layerParameter);


    ::Effekseer::Matrix44 projectionMatrix;
	// projectionMatrix.PerspectiveFovRH_OpenGL(90.0f / 180.0f * 3.14159f, windowSizeX / windowSizeY, 1.0f, 500.0f);
	projectionMatrix.PerspectiveFovRH_OpenGL(fovRadians, (float)windowSizeX / (float)windowSizeY, nearPlane, farPlane);

    ::Effekseer::Matrix44 cameraMatrix;
    cameraMatrix.LookAtRH(viewerPosition, lookAt, ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f));

	effekRenderer->SetProjectionMatrix(projectionMatrix);
	effekRenderer->SetCameraMatrix(cameraMatrix);


	Effekseer::Manager::DrawParameter drawParameter;
    drawParameter.ZNear = nearPlane;
    drawParameter.ZFar = farPlane;

	drawParameter.ViewProjectionMatrix = effekRenderer->GetCameraProjectionMatrix();

	effekRenderer -> BeginRendering();
	effekseerManager -> Draw(drawParameter);
	effekRenderer -> EndRendering();
}

Effekseer::EffectRef doCreateEffect(std::string& effect){
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    auto pathUtf16 = convert.from_bytes(effect.c_str());
	Effekseer::EffectRef effectRef = Effekseer::Effect::Create(effekseerManager,  pathUtf16.c_str());
	modassert(effectRef != NULL, std::string("effect invalid: ") + effect);
	return effectRef;
}

EffekEffect createEffect(std::string effect, glm::vec3 position, glm::quat rotation){
	auto effectId = getUniqueObjId();

	effekseerData[effectId] = EffectData {
		.effectRef = doCreateEffect(effect),
		.playingEffect = std::nullopt,
		.position = std::nullopt,
		.rotation = std::nullopt,
		.loopContinuously = false,
		.effectName = effect,
	};

	EffekEffect effekEffect {
		.effectId = effectId,
	};
	updateEffectPosition(effekEffect, position, rotation);
	return effekEffect;
}

void freeEffect(EffekEffect& effect){
	effekseerData.erase(effect.effectId);
}

void setEffectState(EffekEffect& effectEffect, bool loopContinuously){
	auto& effect = effekseerData.at(effectEffect.effectId);
	effect.loopContinuously = loopContinuously;
}

void playEffect(EffekEffect& effect, glm::vec3 position){
	stopEffect(effect);

	auto& effectData = effekseerData.at(effect.effectId);
	Effekseer::Handle playingEffect = effekseerManager -> Play(effectData.effectRef, position.x, position.y, position.z);
	effectData.playingEffect = playingEffect;
	effectData.position = position;
}

void stopEffect(EffekEffect& effectEffect){
	auto& effect = effekseerData.at(effectEffect.effectId);
	if (!effect.playingEffect.has_value()){
		return;
	}
	effekseerManager->StopEffect(effect.playingEffect.value());
}

void updateEffectPosition(EffekEffect& effectEffect, glm::vec3 position, glm::quat rotation){
	auto& effect = effekseerData.at(effectEffect.effectId);
	effect.position = position;
	effect.rotation = rotation;
	if (effect.playingEffect.has_value()){
		effekseerManager->SetLocation(effect.playingEffect.value(), position.x, position.y, position.z);

		glm::vec3 euler = glm::eulerAngles(rotation);
		effekseerManager->SetRotation(
		    effect.playingEffect.value(),
		    euler.x,
		    euler.y,
		    euler.z
		);
	}
}

void reloadEffect(std::string file){
	modlog("effekseer reloadEffect", file);
	for (auto& [id, effect] : effekseerData){
		if (true || effect.effectName == file){ // paths are wrong, just reload them all for now
			modlog("effekseer reload effect", effect.effectName);
			if (effect.playingEffect.has_value()){
				effekseerManager -> StopEffect(effect.playingEffect.value());
				effect.playingEffect = std::nullopt;
			}
			effect.effectRef = NULL;
			effect.effectRef = doCreateEffect(effect.effectName);
		}
	}
}


int effekSeekerTriangleCount(){
	return effekRenderer -> GetDrawVertexCount() / 3;
}

int effekSeekerDrawCount(){
	return effekRenderer -> GetDrawCallCount();
}

