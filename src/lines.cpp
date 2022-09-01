#include "./lines.h"

LineData createLines(){
  return LineData {
    .lineColors = {},
    .text = {},
  };
}

LineByColor* lineByColorOrMakeNew(LineData& lineData, glm::vec4 color, unsigned int linewidth){
  for (auto &lineByColor : lineData.lineColors){
    if (aboutEqual(color, lineByColor.tint) && lineByColor.linewidth == linewidth){
      return &lineByColor;
    }
  }
  lineData.lineColors.push_back(LineByColor{
    .tint = color,
    .linewidth = linewidth,
    .lines = {},
  });    
  
  return &lineData.lineColors.at(lineData.lineColors.size() - 1);
}
objid addLineToNextCycleTint(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth){
  auto lineId = getUniqueObjId();
  auto color = tint.has_value() ? tint.value() : glm::vec4(0.f, 1.f, 0.f, 1.f);
  auto lineWidth = linewidth.has_value() ? linewidth.value() : 5;
  LineByColor* linesByColor = lineByColorOrMakeNew(lineData, color, lineWidth);
  assert(linesByColor != NULL);

  linesByColor -> lines.push_back(
    LineDrawingOptions {
      .line = Line{
        .fromPos = fromPos,
        .toPos = toPos,
      },
      .lineid = lineId,
      .owner = owner, 
      .textureId = textureId,
      .permaLine = permaline,
    }
  );   
  return lineId;
}

objid addLineToNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color, std::optional<unsigned int> textureId){
  std::optional<glm::vec4> tint = std::nullopt;
  if (color == GREEN){
    tint = glm::vec4(0.f, 1.f, 0.f, 1.f);
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, tint, textureId, std::nullopt);
  }else if (color == BLUE){
    tint = glm::vec4(0.f, 0.f, 1.f, 1.f);
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, tint, textureId, std::nullopt);
  }else if (color == RED){
    tint = glm::vec4(1.f, 0.f, 0.f, 1.f);
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, tint, textureId, std::nullopt);
  }
  assert(false);
  return 0;
}

void freeLine(LineData& lineData, objid lineId){
  std::vector<LineByColor> lineColors;
  for (auto &lineByColor : lineData.lineColors){
    std::vector<LineDrawingOptions> newLines;
    for (auto &line : lineByColor.lines){
      if (lineId != line.lineid){
        newLines.push_back(line);
      }
    }
    if (newLines.size() > 0){
      lineColors.push_back(LineByColor{
        .tint = lineByColor.tint,
        .linewidth = lineByColor.linewidth,
        .lines = newLines,
      });
    }
  }
  lineData.lineColors = lineColors;
}
 
void removeLinesByOwner(LineData& lineData, objid owner){
  std::vector<LineByColor> lineColors;
  for (auto &lineByColor : lineData.lineColors){
    std::vector<LineDrawingOptions> newLines;
    for (auto &line : lineByColor.lines){
      if (owner != line.owner){
        newLines.push_back(line);
      }
    }
    if (newLines.size() > 0){
      lineColors.push_back(LineByColor{
        .tint = lineByColor.tint,
        .linewidth = lineByColor.linewidth,
        .lines = newLines,
      });
    }
  }
  lineData.lineColors = lineColors;
}
void removeTempText(LineData& lineData){
  std::vector<TextDrawingOptions> newText;
  for (auto &text : lineData.text){
    if (text.permaText){
      newText.push_back(text);
    }
  }
  lineData.text.clear();
  lineData.text = newText;
}
void removeTempLines(LineData& lineData){
  std::vector<LineByColor> lineColors;
  for (auto &lineByColor : lineData.lineColors){
    std::vector<LineDrawingOptions> newLines;
    for (auto &line : lineByColor.lines){
      if (line.permaLine){
        newLines.push_back(line);
      }
    }
    if (newLines.size() > 0){
      lineColors.push_back(LineByColor{
        .tint = lineByColor.tint,
        .linewidth = lineByColor.linewidth,
        .lines = newLines,
      });
    }
  }
  lineData.lineColors = lineColors;
}


bool textureIdSame(std::optional<unsigned int> lineTexture, std::optional<unsigned int> textureId){
  return (
    (!lineTexture.has_value() && !textureId.has_value()) ||
    (lineTexture.has_value() && textureId.has_value() && lineTexture.value() == textureId.value())
  );
}


void drawAllLines(LineData& lineData, GLint shaderProgram, std::optional<unsigned int> textureId){
  for (auto &lineByColor : lineData.lineColors){
    std::vector<Line> lines;
    for (auto &line : lineByColor.lines){
      if (textureIdSame(line.textureId, textureId)){
        lines.push_back(line.line);
      }
    }
    if (lines.size() > 0){
      glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(lineByColor.tint));
      drawLines(lines, lineByColor.linewidth); 
    }
  }
}

void addTextData(LineData& lineData, TextDrawingOptions text){
  lineData.text.push_back(text);
}


glm::vec2 convertTextNDICoords(float left, float top, float height, float width, bool isndi){
  if (isndi){
    return glm::vec2(left, top);
  }
  float heightFromTop = height - top;
  return pixelCoordToNdi(glm::ivec2(left, heightFromTop), glm::vec2(width, height));
}

float convertTextNdiFontsize(float height, float width, float fontsize, bool isndi){
  return fontsize;
}

void drawTextData(LineData& lineData, unsigned int uiShaderProgram, std::function<FontFamily&(std::string)> fontFamilyByName, std::optional<unsigned int> textureId, unsigned int height, unsigned int width){
  //std::cout << "text number: " << lineData.text.size() << std::endl;
  for (auto &text : lineData.text){
    if (textureIdSame(text.textureId, textureId)){
      //std::cout << "drawing words: " << text.word << std::endl;
      glUniform4fv(glGetUniformLocation(uiShaderProgram, "tint"), 1, glm::value_ptr(text.tint));
      auto coords = convertTextNDICoords(text.left, text.top, height, width, text.ndi);
      auto adjustedFontSize = convertTextNdiFontsize(height, width, text.fontSize, text.ndi);
      FontFamily& fontFamily = fontFamilyByName(text.fontFamily.has_value() ? text.fontFamily.value() : "");
      if (text.selectionId.has_value()){
        //std::cout << "selection id value: " << text.selectionId.value() << std::endl;
        auto id = text.selectionId.value();
        auto color = getColorFromGameobject(id);

        Color colorTypeColor {
          .r = color.x,
          .g = color.y, 
          .b = color.z,
          .a = color.w,
        };
        auto restoredId = getIdFromColor(colorTypeColor);
        //std::cout << "color is: " << print(colorTypeColor) << " - " << id << " - " << restoredId << std::endl;
        glUniform4fv(glGetUniformLocation(uiShaderProgram, "encodedid2"), 1, glm::value_ptr(getColorFromGameobject(id)));

      }else{
        glUniform4fv(glGetUniformLocation(uiShaderProgram, "encodedid2"), 1, glm::value_ptr(getColorFromGameobject(0)));
      }
      drawWords(uiShaderProgram, fontFamily, text.word, coords.x, coords.y, adjustedFontSize);  
    }
  }
}
void disposeTempBufferedData(LineData& lineData){
  removeTempText(lineData);
  removeTempLines(lineData);
}

