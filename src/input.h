#ifndef MOD_INPUT
#define MOD_INPUT

#include <GLFW/glfw3.h>
#include <iostream>
#include "./state.h"

void handleInput(GLFWwindow *window, float deltaTime, 
	engineState& state, 
	void (*translate)(float, float, float), void (*scale)(float, float, float), void (*rotate)(float, float, float),
	void (*moveCamera)(glm::vec3), void (*nextCamera)(void),
	void (*playSound)(void)
);

void onMouseEvents(GLFWwindow* window, double xpos, double ypos);

#endif