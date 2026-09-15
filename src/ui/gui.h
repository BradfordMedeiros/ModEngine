#pragma once

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "./widgets/core.h"
#include "./widgets/widgets.h"
#include "./widgets/obj.h"



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
void renderSplitLayout(ViewMenuItem& view, glm::vec2 additionalOffset = glm::vec2(0.f, 0.f));

void renderWidget2(WidgetMenuItem2& item, bool includePanel);
std::optional<WidgetMenuItem2*> widgetByNameSymbol(int symbol);


void renderLayoutAlignUpCenterHorz(const char* name, WidgetMenuItem2& widget, ImVec2 ndi, ImVec2 alignment, ImVec2 size, ImVec4 color = ImVec4(0.f, 0.f, 0.f, 0.f));
void renderLayoutCenter(const char* name, WidgetMenuItem2& widget);
void renderLayoutHalf(WidgetMenuItem2& widgetOne, WidgetMenuItem2& widgetTwo);


void drawImGuiText(std::string text, std::optional<glm::vec2> positionNdi);
void clearImGuiData();

