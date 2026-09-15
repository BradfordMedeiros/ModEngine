#include "./resource.h"

std::vector<ImGuiColor> imguiColors;
std::vector<ImGuiLoadedFont> imguiFonts;
std::vector<ImGuiFontBinding> imguiFontBindings;

ImVec4 getImGuiColor(int symbol, glm::vec4 defaultColor){
  for (auto& color : imguiColors){
    if (color.symbol == symbol){
      return ImVec4(color.color.r, color.color.g, color.color.b, color.color.a);
    }
  }
  imguiColors.push_back(ImGuiColor{
    .symbol = symbol,
    .color = defaultColor,
  });
  return ImVec4(defaultColor.r, defaultColor.g, defaultColor.b, defaultColor.a);
}


void loadImGuiFont(int symbol, std::string path, float fontSize){
  for (auto& font : imguiFonts){
    if (font.symbol == symbol){
      return;
    }
  }

  ImGuiIO& io = ImGui::GetIO();
  ImFont* loadedFont = io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize);
  imguiFonts.push_back(ImGuiLoadedFont {
    .font = loadedFont,
    .symbol = symbol,
    .fontSize = fontSize,
    .path = path,
  });
}


static int defaultFontSymbol = getSymbol("default-font");
ImGuiLoadedFont* getImGuiDefaultFontType(){
  for (auto& font : imguiFonts){
    if (font.symbol == defaultFontSymbol){
      return &font;
    }
  }
  return NULL;
}
ImGuiLoadedFont& getImGuiFontType(int symbol){
  for (auto& font : imguiFonts){
    if (font.symbol == symbol){
      return font;
    }
  }

  auto fontPtr = getImGuiDefaultFontType();
  modassert(fontPtr, "no default font");
  return *fontPtr;
}

ImFont* getImGuiFont(int symbol){
  auto& font = getImGuiFontType(symbol);
  return font.font;
}


void loadImGuiFontBinding(int symbol, int fontSymbol){
	imguiFontBindings.push_back(ImGuiFontBinding{
		.fontBinding = symbol,
		.fontSymbol = fontSymbol,
	});
}

ImFont* getFontByBinding(int symbol){
  for (auto& imguiFontBinding : imguiFontBindings){
    if (imguiFontBinding.fontBinding == symbol){
      return getImGuiFont(imguiFontBinding.fontSymbol);
    } 
  }
  imguiFontBindings.push_back(ImGuiFontBinding {
    .fontBinding = symbol,
    .fontSymbol = defaultFontSymbol,
  });
  return getImGuiFont(defaultFontSymbol);
}

bool loadUiData(std::string filepath){
  auto fileContent = readFileOrPackage(filepath);
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
  if (doc.HasParseError()){
    std::cout << "error parsing ui file: " << filepath << "  (" << fileContent << ")" << std::endl;
    exit(0);
  }

  {
    auto it = doc.FindMember("colors");
    if (it != doc.MemberEnd() && it->value.IsObject()){
      auto& colors = it->value;

      for (auto colorIt = colors.MemberBegin(); colorIt != colors.MemberEnd(); ++colorIt){
        if (colorIt->value.IsArray() && colorIt->value.Size() == 4){
          auto symbol = getSymbol(colorIt->name.GetString());
          auto color = glm::vec4(colorIt->value[0].GetFloat(), colorIt->value[1].GetFloat(), colorIt->value[2].GetFloat(), colorIt->value[3].GetFloat());

          imguiColors.push_back(ImGuiColor{
            .symbol = symbol,
            .color = color,
          });
        }
      }
    }
  }

  {
    auto it = doc.FindMember("fonts");
    if (it != doc.MemberEnd() && it->value.IsObject()){
      auto& fonts = it->value;

      for (auto fontIt = fonts.MemberBegin(); fontIt != fonts.MemberEnd(); ++fontIt){
        auto& font = fontIt->value;

        auto symbol = getSymbol(fontIt->name.GetString());
        auto file = font["file"].GetString();
        auto size = font["size"].GetFloat();

        loadImGuiFont(symbol, file, size);
      }
    }
  }

  {
    auto it = doc.FindMember("font-bindings");
    if (it != doc.MemberEnd() && it->value.IsObject()){
      auto& fonts = it->value;

      for (auto fontIt = fonts.MemberBegin(); fontIt != fonts.MemberEnd(); ++fontIt){
        auto& font = fontIt->value;
        auto symbol = getSymbol(fontIt->name.GetString());
        auto fontSymbol = defaultFontSymbol;
        if(font.HasMember("font") && font["font"].IsString()){
          auto fontName = font["font"].GetString();
          fontSymbol = getSymbol(fontName);
        }
        loadImGuiFontBinding(symbol, fontSymbol);
      }
    }
  }

  return false;
}

void saveUiData(std::string filepath){
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

  {
    rapidjson::Value jsonMap(rapidjson::kObjectType);
    for (auto& imguiColor : imguiColors) {
        auto colorName = nameForSymbol(imguiColor.symbol);
        rapidjson::Value key(colorName, allocator);
        rapidjson::Value value(rapidjson::kArrayType);
        value.PushBack(imguiColor.color.r, allocator);
        value.PushBack(imguiColor.color.g, allocator);
        value.PushBack(imguiColor.color.b, allocator);
        value.PushBack(imguiColor.color.a, allocator);
        jsonMap.AddMember(key, value, allocator);
    }
    doc.AddMember("colors", jsonMap, allocator);
  }

  {
    rapidjson::Value jsonMap(rapidjson::kObjectType);
    for (auto& imguiFont : imguiFonts) {
        auto fontName = nameForSymbol(imguiFont.symbol);
        rapidjson::Value name(fontName, allocator);
        rapidjson::Value fontObject(rapidjson::kObjectType);
        rapidjson::Value fontPath(imguiFont.path, allocator);
        fontObject.AddMember("file", fontPath, allocator);
        fontObject.AddMember("size", imguiFont.fontSize, allocator);
        jsonMap.AddMember(name, fontObject, allocator);
    }
    doc.AddMember("fonts", jsonMap, allocator);
  }

  {
    rapidjson::Value jsonMap(rapidjson::kObjectType);
    for (auto& fontBinding : imguiFontBindings) {
        auto bindingName = nameForSymbol(fontBinding.fontBinding);
        rapidjson::Value name(bindingName, allocator);

        rapidjson::Value fontObject(rapidjson::kObjectType);
        auto fontName = nameForSymbol(fontBinding.fontSymbol);
        rapidjson::Value fontValue(fontName, allocator);

        fontObject.AddMember("font", fontValue, allocator);
        jsonMap.AddMember(name, fontObject, allocator);
    }
    doc.AddMember("font-bindings", jsonMap, allocator);
  }

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  auto strValue = buffer.GetString();
  std::cout << strValue << std::endl;

  realfiles::saveFile(filepath, strValue);
}