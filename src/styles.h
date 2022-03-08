#ifndef MOD_STYLES
#define MOD_STYLES

#include <vector>
#include <string>
#include <set>
#include "./scene/serialization.h"

/*
platform:tint:10 10 10  #any element named platform
.class:tint:1 0 0    # any element with attribute class gets an attribute tint, but does not overwrite the existing value
.!class:tint:1 0 0   # any element with attribute class gets an attribute tint, overwrites existing value

,red=1 0 0:tint:1 0 0  # any element with payload = red with value 1 0 0, gets tint
,red=1 0 0       # any element with payload
*/

enum StyleSelectorType { STYLE_SELECTOR_NAME, STYLE_SELECTOR_ATTRIBUTE, STYLE_SELECTOR_PAYLOAD };
struct StyleSelectorQuery {
  StyleSelectorType type;
  std::string target;
};
struct Style {
  std::vector<StyleSelectorQuery> queries;
  std::string attribute;
  std::string payload;
};

std::vector<Style> loadStyles(std::string filepath);
void applyStyles(std::vector<Token>& tokens, std::vector<Style>& styles);

#endif