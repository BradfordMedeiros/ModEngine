#ifndef MOD_OBJ_UILAYOUT
#define MOD_OBJ_UILAYOUT

#include "../../common/util.h"
#include "../common/util/boundinfo.h"
#include "./obj_util.h"

enum UILayoutType { LAYOUT_HORIZONTAL, LAYOUT_VERTICAL };

enum UILayoutMinSizeType { UILayoutNone, UILayoutPercent };
struct UILayoutMinSize {
  bool hasMinSize;
  UILayoutMinSizeType type;
  float amount;
};
enum UILayoutFlowType { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive };

struct LayoutMargin {
  float margin;
  float marginLeft;
  float marginRight;
  float marginBottom;
  float marginTop;
  bool marginSpecified;
  bool marginLeftSpecified;
  bool marginRightSpecified;
  bool marginBottomSpecified;
  bool marginTopSpecified;
};

struct LayoutAnchor {
  std::string target;
  glm::vec3 offset;
  UILayoutFlowType horizontal;
  UILayoutFlowType vertical;
};
struct LayoutBorder {
  float borderSize;
  glm::vec4 borderColor;
  bool hasBorder;
};

enum LayoutContentAlignmentType { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive };
struct LayoutContentAlignment {
  LayoutContentAlignmentType vertical;
  LayoutContentAlignmentType horizontal;
};

enum LayoutContentSpacing  { LayoutContentSpacing_Pack, LayoutContentSpacing_SpaceForFirst, LayoutContentSpacing_SpaceForLast };

struct GameObjectUILayout {
  UILayoutType type;
  float spacing;
  float minSpacing;
  std::vector<std::string> elements;
  BoundInfo boundInfo;
  glm::vec3 panelDisplayOffset;
  bool showBackpanel;
  glm::vec4 tint;
  LayoutMargin marginValues;
  LayoutAnchor anchor;
  TextureInformation texture;
  UILayoutMinSize minwidth;
  UILayoutMinSize minheight;
  UILayoutFlowType horizontal;
  UILayoutFlowType vertical;
  LayoutBorder border;
  LayoutContentAlignment alignment;
  LayoutContentAlignmentType contentAlign;
  LayoutContentSpacing contentSpacing;
};

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeLayout(GameObjectUILayout& obj, ObjectSerializeUtil& util);
glm::mat4 layoutBackpanelModelTransform(GameObjectUILayout& layoutObj, glm::vec3 minusScale, glm::vec3 layoutPos);

void getUILayoutAttributes(GameObjectUILayout& layoutObj, GameobjAttributes& _attributes);
void setUILayoutAttributes(GameObjectUILayout& layoutObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif