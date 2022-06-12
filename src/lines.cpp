#include "./lines.h"

LineData createLines(){
  return LineData {
    .lineColors = {},
    .text = {},
  };
}

LineByColor* lineByColorOrMakeNew(LineData& lineData, glm::vec4 color){
  for (auto &lineByColor : lineData.lineColors){
    if (aboutEqual(color, lineByColor.tint)){
      return &lineByColor;
    }
  }
  lineData.lineColors.push_back(LineByColor{
    .tint = color,
    .lines = {},
  });    
  
  return &lineData.lineColors.at(lineData.lineColors.size() - 1);
}
objid addLineToNextCycleTint(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId){
  auto lineId = getUniqueObjId();
  auto color = tint.has_value() ? tint.value() : glm::vec4(0.f, 1.f, 0.f, 1.f);
  LineByColor* linesByColor = lineByColorOrMakeNew(lineData, color);
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
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, tint, textureId);
  }else if (color == BLUE){
    tint = glm::vec4(0.f, 0.f, 1.f, 1.f);
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, tint, textureId);
  }else if (color == RED){
    tint = glm::vec4(1.f, 0.f, 0.f, 1.f);
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, tint, textureId);
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
  std::cout << "line by color size: " << lineData.lineColors.size() << std::endl;
  for (auto &lineByColor : lineData.lineColors){
    std::vector<Line> lines;
    for (auto &line : lineByColor.lines){
      if (textureIdSame(line.textureId, textureId)){
        lines.push_back(line.line);
      }
    }
    if (lines.size() > 0){
      glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(lineByColor.tint));
      drawLines(lines); 
    }
  }
}

void addTextData(LineData& lineData, TextDrawingOptions text){
  lineData.text.push_back(text);
}

void drawTextData(LineData& lineData, unsigned int uiShaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::optional<unsigned int> textureId, unsigned int height){
  //std::cout << "text number: " << lineData.text.size() << std::endl;
  for (auto &text : lineData.text){
    if (textureIdSame(text.textureId, textureId)){
      //std::cout << "drawing words: " << text.word << std::endl;
      glUniform4fv(glGetUniformLocation(uiShaderProgram, "tint"), 1, glm::value_ptr(text.tint));
      drawWords(uiShaderProgram, fontMeshes, text.word, text.left, height - text.top, text.fontSize);  
    }
  }
}
void disposeTempBufferedData(LineData& lineData){
  removeTempText(lineData);
  removeTempLines(lineData);
}

