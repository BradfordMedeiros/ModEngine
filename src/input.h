#ifndef MOD_INPUT
#define MOD_INPUT

#include <GLFW/glfw3.h>
#include <iostream>
#include "./state.h"
#include "./common/util.h"
#include "./common/profiling.h"
#include "./main_api.h"
#include "./gizmo/keymapper.h"

void mouse_button_callback(bool disableInput, GLFWwindow* window, engineState& state, int button, int action, int mods,  void (*handleSerialization) (void));
void onMouse(bool disableInput, GLFWwindow* window, engineState& state, double xpos, double ypos, void(*rotateCamera)(float, float));
void scroll_callback(GLFWwindow *window, engineState& state, double xoffset, double yoffset);

struct JoyStickInfo {
  int index;
  float value;
};

void handleInput(
  KeyRemapper& remapper,
  bool disableInput, 
  GLFWwindow *window, 
  float deltaTime, 
	engineState& state, 
	void (*moveCamera)(glm::vec3), void (*nextCamera)(void),
	void (*setObjectDimensions)(int32_t index, float width, float height, float depth),
  void (*onDebugKey)(),
  void (*onArrowKey)(int key),
  void (*onCameraSystemChange)(bool usingBuiltInCamera),
  void (*onDelete)(),
  void (*onKeyChar)(unsigned int codepoint),
  void (*onJoystick)(std::vector<JoyStickInfo> infos)
);

#endif
