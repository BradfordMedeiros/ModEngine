#ifndef MOD_MAININPUT
#define MOD_MAININPUT

#include <GLFW/glfw3.h>
#include "./scene/scene.h"
#include "./main_api.h"
#include "./easyuse/easy_use.h"
#include "./easyuse/editor.h"
#include "./keymapper.h"
#include "./drawing.h"
#include "./colorselection.h"
#include "./benchmark.h"
#include "./common/profiling.h"
#include "./easyuse/manipulator.h"
#include "./worldchunking.h"
#include "./scene/scene_offline.h"
#include "./renderstages.h"
#include "./scene/styles.h"
#include "./lines.h"

struct JoyStickInfo {
  int index;
  float value;
};

std::string dumpDebugInfo(bool fullInfo = true);
void debugInfo(std::string infoType, std::string filepath);

glm::mat4 projectionFromLayer(LayerInfo& layer);
LayerInfo layerByName(std::string layername);
LayerInfo getLayerForId(objid id);

void onMouseEvents(GLFWwindow* window, double xpos, double ypos);
void onMouseCallback(GLFWwindow* window, int button, int action, int mods);
void mouse_button_callback(bool disableInput, GLFWwindow* window, engineState& state, int button, int action, int mods,  void (*handleSerialization) (void));
void joystickCallback(int jid, int event);
void onJoystick(std::vector<JoyStickInfo> infos);
void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void keyCharCallback(unsigned int codepoint);
void keyCharCallback(GLFWwindow* window, unsigned int codepoint);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButton();
void drop_callback(GLFWwindow* window, int count, const char** paths);
void processControllerInput(KeyRemapper& remapper, void (*moveCamera)(glm::vec3), float deltaTime,  void (*onKeyChar)(unsigned int codepoint), void (*onJoystick)(std::vector<JoyStickInfo> infos));
void processKeyBindings(GLFWwindow *window, KeyRemapper& remapper);
void toggleFullScreen(bool fullscreen);
void toggleCursor(bool focusCursor);

void handleInput(GLFWwindow* window);

#endif