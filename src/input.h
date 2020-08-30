#ifndef MOD_INPUT
#define MOD_INPUT

#include <GLFW/glfw3.h>
#include <iostream>
#include "./state.h"
#include "./common/util.h"

void mouse_button_callback(bool disableInput, GLFWwindow* window, engineState& state, int button, int action, int mods,  void (*handleSerialization) (void));
void onMouse(bool disableInput, GLFWwindow* window, engineState& state, double xpos, double ypos, void(*rotateCamera)(float, float));
void scroll_callback(GLFWwindow *window, engineState& state, double xoffset, double yoffset);

void handleInput(bool disableInput, GLFWwindow *window, float deltaTime, 
	engineState& state, 
	void (*moveCamera)(glm::vec3), void (*nextCamera)(void),
	void (*setObjectDimensions)(short index, float width, float height, float depth),
  void (*onDebugKey)(),
  void (*onArrowKey)(int key),
  void (*onCameraSystemChange)(bool usingBuiltInCamera),
  void (*onDelete)()
);

#endif
