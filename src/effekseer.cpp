#include "./effekseer.h"

double timeElapsed();


Effekseer::ManagerRef effekseerManager;
EffekseerRendererGL::RendererRef effekRenderer;

std::optional<Effekseer::Handle> playingEffect;
Effekseer::EffectRef effect;

void initEffekseer(){
  std::cout << "effekseer initialized start" << std::endl;

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


  effect = Effekseer::Effect::Create(effekseerManager, u"./res/particles/Laser01.efkefc");
  if (effect == NULL){
  	std::cout << "effekseer effect could not be loaded" << std::endl;
  	exit(1);
  }

  playingEffect = effekseerManager -> Play(effect, 0.0f, 0.0f, 0.0f);
  if (playingEffect.value() == -1){
  	std::cout << "effekseer could not play the effect" << std::endl;
  	exit(1);
  }

  std::cout << "effekseer initialized end" << std::endl;

}
void onEffekSeekerFrame(){
	static bool initialized = false;

	if (!initialized){
		initialized = true;


		initEffekseer();
	}

	bool effectStillPlaying = effekseerManager->Exists(playingEffect.value());
	std::cout << "effect playing: " << effectStillPlaying << std::endl;

	if (!effectStillPlaying){
	  playingEffect = effekseerManager -> Play(effect, 0.0f, 0.0f, 0.0f);
	  std::cout << "effect reset" << std::endl;
  	  if (playingEffect.value() == -1){
  	  	std::cout << "effekseer effect could not be replayed" << std::endl;
  	  	exit(1);
  	  }
	}
 	
	effekseerManager -> AddLocation(playingEffect.value(), ::Effekseer::Vector3D(0.2f, 0.0f, 0.0f));


	//auto effect = Effekseer::Effect::Create(efkManager, EFK_EXAMPLE_ASSETS_DIR_U16 "Laser01.efkefc");
	//static int time = 0;
	//time++;
	//if (time % 120 == 0){
	//	efkHandle = effekseerManager ->Play(effect, 0, 0, 0);
//
	//}
	//if (time % 120 == 119)
	//{
	//	// Stop effects
	//	// エフェクトの停止
	//	effekseerManager -> StopEffect(efkHandle);
	//}

	///*	\~English	Update all effects.
	//	\~Japanese	全てのエフェクトの更新処理を行う。
	//	@param	deltaFrame
	//	\~English	passed time (1 is 1/60 seconds)
	//	\~Japanese	更新するフレーム数(60fps基準)
	//*/
	//virtual void Update(float deltaFrame = 1.0f) = 0;
	Effekseer::Manager::UpdateParameter updateParameter;

	effekseerManager -> Update(); // pass in the actual delta time

	for (int i = 0; i < 100000; i++){
		std::cout << "hasdf" << std::endl;
	}
}


void onEffekSeekerRender(float windowSizeX, float windowSizeY){
	std::cout << "effekseer: " << windowSizeX << ", " << windowSizeY << std::endl;

	auto viewerPosition = ::Effekseer::Vector3D(10.0f, 5.0f, 10.0f);
    auto lookAt = ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f);

	Effekseer::Manager::LayerParameter layerParameter;
	layerParameter.ViewerPosition = viewerPosition;
	
	effekseerManager -> SetLayerParameter(0, layerParameter);


    ::Effekseer::Matrix44 projectionMatrix;
//    projectionMatrix.PerspectiveFovRH_OpenGL(90.0f / 180.0f * 3.14159f, windowSizeX / windowSizeY, 1.0f, 500.0f);
	projectionMatrix.PerspectiveFovRH(90.0f / 180.0f * 3.14f, (float)windowSizeX / (float)windowSizeY, 1.0f, 500.0f);

    ::Effekseer::Matrix44 cameraMatrix;
    cameraMatrix.LookAtRH(viewerPosition, lookAt, ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f));

		// 投影行列を設定
	effekRenderer->SetProjectionMatrix(projectionMatrix);

		// Specify a camera matrix
		// カメラ行列を設定
	effekRenderer->SetCameraMatrix(cameraMatrix);


	Effekseer::Manager::DrawParameter drawParameter;
	drawParameter.ZNear = 0.0f;
	drawParameter.ZFar = 1.0f;
    drawParameter.ZNear = 0.0001f;
    drawParameter.ZFar = 500.0f;

	drawParameter.ViewProjectionMatrix = effekRenderer->GetCameraProjectionMatrix();

	effekRenderer -> BeginRendering();
	effekseerManager -> Draw(drawParameter);
	effekRenderer -> EndRendering();
}

/*

void RenderParticles(const glm::mat4& projection, const glm::mat4& view) {
    // Effekseer wants column-major matrices
    Effekseer::Matrix44 proj, view44;
    memcpy(proj.Values, &projection[0][0], sizeof(float) * 16);
    memcpy(view44.Values, &view[0][0], sizeof(float) * 16);

    particleManager->GetRenderer()->SetProjectionMatrix(proj);
    particleManager->GetRenderer()->SetCameraMatrix(view44);

    particleManager->GetManager()->Draw();
}
*/