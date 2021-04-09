#include <functional>
#include "../common/util.h"
#include "../scene/serialobject.h"  // todo prob dont depend on gameobj directly

void onManipulatorSelectItem(objid selectedItem, std::string selectedItemName, std::function<objid(void)> makeManipulator, std::function<void(objid)> removeObjectById, std::function<GameObject&(objid)> getGameObject);
void onManipulatorMouseRelease(std::function<GameObject&(objid)> getGameObject);
void onManipulatorUpdate(std::function<GameObject&(objid)> getGameObject, float mouseX, float mouseY);