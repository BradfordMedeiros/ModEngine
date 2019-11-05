#ifndef MOD_INPUT
#define MOD_INPUT

#include <GLFW/glfw3.h>
#include <iostream>
#include "./state.h"
#include "./common/util.h"

void mouse_button_callback(GLFWwindow* window, engineState& state, int button, int action, int mods, 
	void (*handleSerialization) (void), void (*selectItem) (void)
);
void onMouse(GLFWwindow* window, engineState& state, double xpos, double ypos, void(*rotateCamera)(float, float));

void handleInput(GLFWwindow *window, float deltaTime, 
	engineState& state, 
	void (*translate)(float, float, float), void (*scale)(float, float, float), void (*rotate)(float, float, float),
	void (*moveCamera)(glm::vec3), void (*nextCamera)(void),
	void (*playSound)(void),
	void (*setObjectDimensions)(short index, float width, float height, float depth),
  void sendMoveObjectMessage(),
  void (*makeObject)(std::string name, std::string meshName, float x, float y, float z)
);

#endif
