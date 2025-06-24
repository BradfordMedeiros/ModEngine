#ifndef MOD_MAININPUT
#define MOD_MAININPUT

#include <GLFW/glfw3.h>
#include "./scene/scene.h"
#include "./main_api.h"
#include "./easyuse/easy_use.h"
#include "./easyuse/editor.h"
#include "./keymapper.h"
#include "./colorselection.h"
#include "./perf/benchmark.h"
#include "./perf/profiling.h"
#include "./easyuse/manipulator.h"
#include "./worldchunking.h"
#include "./scene/scene_offline.h"
#include "./renderstages.h"
#include "./lines.h"
#include "./main_util.h"
#include "./scene/animation/timeplayback.h"
#include "./modlayer.h"
#include "./world_tasks.h"
#include "./common/symbols.h"

struct JoyStickInfo {
  int index;
  float value;
};

std::string dumpDebugInfo(bool fullInfo = true);
void debugInfo(std::optional<std::string> filepath);

glm::vec2 ndiCoord();
void onMouseEvents(GLFWwindow* window, double xpos, double ypos);
void handleMouseEvents();

void onMouseCallback(GLFWwindow* window, int button, int action, int mods);
void dispatchClick(int button, int action);
void moveMouse(glm::vec2 ndi);

void mouse_button_callback(engineState& state, int button, int action, int mods,  void (*handleSerialization) (void));
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
void toggleCursor(CURSOR_TYPE cursorBehavior);

void handleInput(GLFWwindow* window);

#endif