#include "./lines.h"

LineData createLines(){
  return LineData {
    .permaLines = {},
    .screenspaceLines = {},
    .text = {},
  };
}

objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color){
  auto lineId = getUniqueObjId();
  lineData.permaLines.push_back(
    LineDrawingOptions {
      .line = Line{
        .fromPos = fromPos,
        .toPos = toPos,
      },
      .lineid = lineId,
      .owner = owner, 
      .color = color,
      .textureId = std::nullopt,
      .permaLine = permaline,
    }
  );   
  return lineId;
}

objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<unsigned int> textureId){
  std::cout << "add line next cycle: " << textureId.has_value() << std::endl;
  if (textureId.has_value()){
    lineData.screenspaceLines.push_back(Line { .fromPos = fromPos, .toPos = toPos });
    return 0;
  }
  return addLineNextCycle(lineData, fromPos, toPos, permaline, owner, GREEN);
}

void freeLine(LineData& lineData, objid lineId){
  std::vector<LineDrawingOptions> newLines;
  for (auto &line : lineData.permaLines){
    if (lineId != line.lineid){
      newLines.push_back(line);
    }
  }
  lineData.permaLines.clear();
  for (auto line : newLines){
    lineData.permaLines.push_back(line);
  }
}
 
void removeLinesByOwner(LineData& lineData, objid owner){
  MODTODO("move all this line stuff behind some single cleaner interface");
  std::vector<LineDrawingOptions> newLines;
  for (auto &line : lineData.permaLines){
    if (owner != line.owner){
      newLines.push_back(line);
    }
  }
  lineData.permaLines.clear();
  for (auto line : newLines){
    lineData.permaLines.push_back(line);
  }
}
void removeTempLines(LineData& lineData){
  std::vector<LineDrawingOptions> newLines;
  for (auto &line : lineData.permaLines){
    if (line.permaLine){
      newLines.push_back(line);
    }
  }
  lineData.permaLines.clear();
  for (auto line : newLines){
    lineData.permaLines.push_back(line);
  }
}


void addToLineList(LineData& lineData, std::vector<Line>& lines, LineColor color, bool permaLine){
  for (auto permaline : lineData.permaLines){
    if (permaline.color == color && permaline.permaLine == permaLine){
      lines.push_back(permaline.line);
    }
  }
}

void drawAllLines(LineData& lineData, GLint shaderProgram, bool permaLine){
  std::vector<Line> redLines;
  std::vector<Line> greenLines;
  std::vector<Line> blueLines;
  addToLineList(lineData, redLines, RED, permaLine);
  addToLineList(lineData, greenLines, GREEN, permaLine);
  addToLineList(lineData, blueLines, BLUE, permaLine);

  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(1.f, 0.f, 0.f, 1.f)));
  drawLines(redLines);
  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.f, 1.f, 0.f, 1.f)));
  drawLines(greenLines);
  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.f, 0.f, 1.f, 1.f)));
  drawLines(blueLines);
}

void drawAllLines(LineData& lineData, GLint shaderProgram){
  drawAllLines(lineData, shaderProgram, true);
  drawAllLines(lineData, shaderProgram, false);
}

void drawScreenspaceLines(LineData& lineData, GLint shaderProgram){
  std::cout << "number of lines being drawn: " << lineData.screenspaceLines.size() << std::endl;
  drawLines(lineData.screenspaceLines, 1);
}

void addTextData(LineData& lineData, TextDrawingOptions text){
  lineData.text.push_back(text);
}


// Currently only handles the text
void drawTextData(LineData& lineData, unsigned int uiShaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::optional<unsigned int> textureId){
  if (!textureId.has_value()){
    for (auto &text : lineData.text){
      if (!text.textureId.has_value()){
        drawWords(uiShaderProgram, fontMeshes, text.word, text.left, text.top, text.fontSize);  
      }
    }
  }else{
    for (auto &text : lineData.text){
      if (text.textureId.has_value() && text.textureId.value() == textureId.value()){
        modassert(false, "drawing texture text not yet supported");
      }
    }
  }
}
void disposeTempBufferedData(LineData& lineData){
  lineData.text.clear();
  removeTempLines(lineData);
}