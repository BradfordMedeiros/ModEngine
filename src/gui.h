#ifndef MOD_GUI
#define MOD_GUI

#define USE_IMGUI

#ifdef USE_IMGUI

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"

#endif

void initUi();
void renderUi();

#endif