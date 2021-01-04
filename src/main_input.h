#ifndef MOD_MAININPUT
#define MOD_MAININPUT

#include <GLFW/glfw3.h>
#include "./state.h"
#include "./scene/scene.h"
#include "./scene/scene_object.h"
#include "./scheme/scriptmanager.h"
#include "./main_api.h"
#include "./input.h"
#include "./easy_use.h"
#include "./gizmo/keymapper.h"
#include "./drawing.h"
#include "./editor.h"
#include "./colorselection.h"

std::string activeTextureName(std::map<std::string, Texture> worldTextures);
void onMouseEvents(GLFWwindow* window, double xpos, double ypos);
void onArrowKey(int key);
void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void keyCharCallback(GLFWwindow* window, unsigned int codepoint);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButton();

#endif