#include "./dialogmove.h"

struct EditorDialogMove {
	std::optional<objid> hoveringObj;
	std::optional<objid> objSelected;

	std::optional<glm::vec2> currNdi;
	std::optional<glm::vec2> ndiMouseDown;
	std::optional<glm::vec3> objposMouseDown;

	bool restrictX;

	std::optional<float> gridSize;
	std::optional<float> gridOffset;
};

extern CustomApiBindings* mainApi;


glm::vec3 calcNdiOffset(EditorDialogMove& dialogMove, glm::vec2 currNdi){
	float diffX = currNdi.x - dialogMove.ndiMouseDown.value().x;
	float diffY = currNdi.y - dialogMove.ndiMouseDown.value().y;
	float newX = diffX + dialogMove.objposMouseDown.value().x;
	float newY = diffY + dialogMove.objposMouseDown.value().y;
	float oldZ = dialogMove.objposMouseDown.value().z;
	return dialogMove.restrictX  ? glm::vec3(newX, dialogMove.objposMouseDown.value().y, oldZ) : glm::vec3(newX, newY, oldZ) ;
}

glm::vec3 calculateSnapToGrid(glm::vec3 ndiOffset, float cellWidth, float gridOffset){
	float xCell = round(ndiOffset.x / cellWidth);
	float yCell = round((ndiOffset.y + gridOffset) / cellWidth);
	float xPos = cellWidth * xCell;
	float yPos = cellWidth * yCell;
	return glm::vec3(xPos, yPos - gridOffset, ndiOffset.z);
}

CScriptBinding cscriptDialogMoveBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/dialogmove", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    EditorDialogMove* dialogMove = new EditorDialogMove;
    dialogMove -> hoveringObj = std::nullopt;
    dialogMove -> objSelected = std::nullopt;
 		dialogMove -> ndiMouseDown = std::nullopt;
 		dialogMove -> objposMouseDown = std::nullopt;

 		auto attr = mainApi -> getGameObjectAttr(id);
 		auto restrictXAttr = getStrAttr(attr, "dialogmove-restrictx");
 		dialogMove -> restrictX = restrictXAttr.has_value() && restrictXAttr.value() == "true";

 		auto args =  mainApi -> getArgs();
 		dialogMove -> gridSize = args.find("gridsize") != args.end() ? std::optional<float>(std::atof(args.at("gridsize").c_str())) : std::nullopt;
 		dialogMove -> gridOffset = args.find("gridoffset") != args.end() ? std::optional<float>(std::atof(args.at("gridoffset").c_str())) : std::nullopt;

		//mainApi -> enforceLayout(id);
    return dialogMove;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    EditorDialogMove* dialogMove = static_cast<EditorDialogMove*>(data);
    delete dialogMove;
  };
  binding.onObjectHover = [](objid scriptId, void* data, int32_t index, bool hoverOn) -> void {
    EditorDialogMove* dialogMove = static_cast<EditorDialogMove*>(data);
  	if (index == scriptId){
  		if (hoverOn){
  			modlog("editor", "hover on dialog: " + std::to_string(index));
  			dialogMove -> hoveringObj = index;
  		}else{
  			modlog("editor", "hover off dialog: " + std::to_string(index));
  			dialogMove -> hoveringObj = std::nullopt;
  		}
  	}
  };
  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
  	EditorDialogMove* dialogMove = static_cast<EditorDialogMove*>(data);
  	std::cout << "button: " << button << ", action = " << action << ", hoveringobj = " << dialogMove -> hoveringObj.has_value() << std::endl;
  	if (button == 0 && action == 0){
  		if (dialogMove -> objSelected.has_value()){
  			modlog("editor", "draggable release dialog move " + std::to_string(dialogMove -> objSelected.value()));	
  			mainApi -> sendNotifyMessage("dialogmove-drag-stop", std::to_string(dialogMove -> objSelected.value()));
  		}
  		dialogMove -> objSelected = std::nullopt;
  		dialogMove -> ndiMouseDown = std::nullopt;
  		dialogMove -> objposMouseDown = std::nullopt;
  	}else if (button == 0 && action == 1 && dialogMove -> hoveringObj.has_value() && dialogMove -> currNdi.has_value()){
 			dialogMove -> objSelected = dialogMove -> hoveringObj.value();
  		modlog("editor", "draggable start dialog move " + std::to_string(dialogMove -> objSelected.value()));
 			mainApi -> sendNotifyMessage("dialogmove-drag-start", std::to_string(dialogMove -> objSelected.value()));
 			dialogMove -> ndiMouseDown = dialogMove -> currNdi.value();
 			dialogMove -> objposMouseDown = mainApi -> getGameObjectPos(dialogMove -> objSelected.value(), true);
  	}
  };

  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void {
  	EditorDialogMove* dialogMove = static_cast<EditorDialogMove*>(data);
  	dialogMove -> currNdi = glm::vec2(xNdc, yNdc);
  	if (dialogMove -> objSelected.has_value()){
  		auto position = calcNdiOffset(*dialogMove, glm::vec2(xNdc, yNdc));
  		if (dialogMove -> gridSize.has_value()){
  			position = calculateSnapToGrid(position, dialogMove -> gridSize.value(), dialogMove -> gridOffset.has_value() ? dialogMove -> gridOffset.value() : 0);
  		}
      mainApi -> setGameObjectPosition(dialogMove -> objSelected.value(), position, true);
  		//mainApi -> enforceLayout(id);
  	}
  };

  return binding;
}

