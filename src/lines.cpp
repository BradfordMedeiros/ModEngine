#include "./lines.h"

LineData createLines(){
  return LineData {
    .lines = {},
    .text = {},
  };
}

objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color, std::optional<unsigned int> textureId){
  auto lineId = getUniqueObjId();
  lineData.lines.push_back(
    LineDrawingOptions {
      .line = Line{
        .fromPos = fromPos,
        .toPos = toPos,
      },
      .lineid = lineId,
      .owner = owner, 
      .color = color,
      .textureId = textureId,
      .permaLine = permaline,
    }
  );   
  return lineId;
}

objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<unsigned int> textureId){
  return addLineNextCycle(lineData, fromPos, toPos, permaline, owner, GREEN, textureId);
}

void freeLine(LineData& lineData, objid lineId){
  std::vector<LineDrawingOptions> newLines;
  for (auto &line : lineData.lines){
    if (lineId != line.lineid){
      newLines.push_back(line);
    }
  }
  lineData.lines.clear();
  for (auto line : newLines){
    lineData.lines.push_back(line);
  }
}
 
void removeLinesByOwner(LineData& lineData, objid owner){
  std::vector<LineDrawingOptions> newLines;
  for (auto &line : lineData.lines){
    if (owner != line.owner){
      newLines.push_back(line);
    }
  }
  lineData.lines.clear();
  for (auto line : newLines){
    lineData.lines.push_back(line);
  }
}
void removeTempText(LineData& lineData){
  std::vector<TextDrawingOptions> newText;
  for (auto &text : lineData.text){
    if (text.permaText){
      newText.push_back(text);
    }
  }
  lineData.text.clear();
  for (auto text : newText){
    lineData.text.push_back(text);
  }
}
void removeTempLines(LineData& lineData){
  std::vector<LineDrawingOptions> newLines;
  for (auto &line : lineData.lines){
    if (line.permaLine){
      newLines.push_back(line);
    }
  }
  lineData.lines.clear();
  for (auto line : newLines){
    lineData.lines.push_back(line);
  } 
}


bool textureIdSame(std::optional<unsigned int> lineTexture, std::optional<unsigned int> textureId){
  return (
    (!lineTexture.has_value() && !textureId.has_value()) ||
    (lineTexture.has_value() && textureId.has_value() && lineTexture.value() == textureId.value())
  );
}
void addToLineList(LineData& lineData, std::vector<Line>& lines, LineColor color, std::optional<unsigned int> textureId){
  for (auto permaline : lineData.lines){
    if (permaline.color == color && textureIdSame(permaline.textureId, textureId)){
      lines.push_back(permaline.line);
    }
  }
}

void drawAllLines(LineData& lineData, GLint shaderProgram, std::optional<unsigned int> textureId){
  std::vector<Line> redLines;
  std::vector<Line> greenLines;
  std::vector<Line> blueLines;
  addToLineList(lineData, redLines, RED, textureId);
  addToLineList(lineData, greenLines, GREEN, textureId);
  addToLineList(lineData, blueLines, BLUE, textureId);

  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(1.f, 0.f, 0.f, 1.f)));
  drawLines(redLines);
  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.f, 1.f, 0.f, 1.f)));
  drawLines(greenLines);
  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.f, 0.f, 1.f, 1.f)));
  drawLines(blueLines);
}

void addTextData(LineData& lineData, TextDrawingOptions text){
  lineData.text.push_back(text);
}

void drawTextData(LineData& lineData, unsigned int uiShaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::optional<unsigned int> textureId){
  for (auto &text : lineData.text){
    if (textureIdSame(text.textureId, textureId)){
      drawWords(uiShaderProgram, fontMeshes, text.word, text.left, text.top, text.fontSize);  
    }
  }
}
void disposeTempBufferedData(LineData& lineData){
  removeTempText(lineData);
  removeTempLines(lineData);
}

