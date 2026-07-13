#include "imgui.h"
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "../../cscript/cscript_binding.h"

#pragma once

std::optional<objid> renderScenegraph(const char* name, bool includePanel);