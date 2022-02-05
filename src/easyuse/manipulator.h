#include <functional>
#include "../common/util.h"
#include "../scene/serialobject.h"  // todo prob dont depend on gameobj directly
#include "../translations.h"

objid getManipulatorId();
void onManipulatorSelectItem(objid selectedItem, std::string selectedItemName, std::function<objid(void)> makeManipulator, std::function<void(objid)> removeObjectById,   std::function<glm::vec3(objid)> getPosition, std::function<void(objid, glm::vec3)> setPosition);
void onManipulatorMouseRelease();
void onManipulatorUpdate(
  std::function<void(glm::vec3, glm::vec3)> drawLine,
  std::function<glm::vec3(objid)> getPosition, 
  std::function<void(objid, glm::vec3)> setPosition, 
  std::function<glm::vec3(objid)> getScale,
  std::function<void(objid, glm::vec3)> setScale,
  glm::mat4 projection,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize
);
void onManipulatorUnselect(std::function<void(objid)> removeObjectById);