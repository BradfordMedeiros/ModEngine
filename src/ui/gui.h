#ifndef MOD_GUI
#define MOD_GUI

#define USE_IMGUI

#ifdef USE_IMGUI

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "./views/core.h"
#include "./widgets.h"

#endif

struct WidgetMenuItem2 {
    int id;
    std::string name;
    std::optional<std::string> list;
    std::function<void(bool includePanel,  std::optional<objid> objectToDetail, std::optional<objid> sceneId)> render;
};

enum ViewType { SPLIT_LAYOUT, DIVIDED_LAYOUT };
struct ViewMenuItem {
    int id;
    bool hide;
    std::string name;
    ViewType type;
    std::vector<WidgetMenuItem2> leftWidgets;
    std::vector<WidgetMenuItem2> rightWidgets;
};

void initUi();
void renderUi();
void registerWidget(std::string name, std::optional<std::string> list, std::function<void(bool includePanel,  std::optional<objid> objectToDetail, std::optional<objid> sceneId)> render);
void registerAction(std::string name, std::string list, std::function<void()> fn);
void setGuiFn(std::optional<std::function<void()>> fn);

void registerView(std::string name, bool hide, std::vector<std::string> leftWidgetStrs, std::vector<std::string> rightWidgetStrs, ViewType viewType);
std::optional<ViewMenuItem*> viewByName(int symbol);
void renderLayout(ViewMenuItem& dynamicView);

void drawImGuiText(std::string text);
void clearImGuiData();


#endif