#include <functional>
#include "../common/util.h"
#include "../scene/serialobject.h"  // todo prob dont depend on gameobj directly

void onManipulatorSelectItem(objid selectedItem, std::string selectedItemName, std::function<objid(void)> makeManipulator, std::function<void(objid)> removeObjectById,   std::function<glm::vec3(objid)> getPosition, std::function<void(objid, glm::vec3)> setPosition);
void onManipulatorMouseRelease();
void onManipulatorUpdate(
  std::function<glm::vec3(objid)> getPosition, 
  std::function<void(objid, glm::vec3)> setPosition, 
  std::function<glm::vec3(objid)> getScale,
  std::function<void(objid, glm::vec3)> setScale,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  float mouseX, 
  float mouseY
);