#include "./lines.h"

LineData createLines(){
  std::vector<Line> lines;
  std::vector<Line> bluelines;
  std::vector<PermaLine> permaLines;
  std::vector<Line> screenspaceLines;

  screenspaceLines.push_back(Line {
    .fromPos = glm::vec3(0.f, 0.f, 0.f),
    .toPos = glm::vec3(.5f, .5f, 0.f),
  });

  return LineData {
    .lines = lines,
    .bluelines = bluelines,
    .permaLines = permaLines,
    .screenspaceLines = screenspaceLines,
  };
}

objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color){
  if (permaline){
    auto lineId = getUniqueObjId();
    lineData.permaLines.push_back(
      PermaLine {
        .line = Line{
          .fromPos = fromPos,
          .toPos = toPos,
        },
        .lineid = lineId,
        .owner = owner, 
        .color = color,
      }
    );   
    return lineId;
  }
  Line line = {
    .fromPos = fromPos,
    .toPos = toPos
  };
  lineData.lines.push_back(line);
  return 0;
}

objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner){
  return addLineNextCycle(lineData, fromPos, toPos, permaline, owner, GREEN);
}


void addScreenspaceLine(LineData& lineData, float xpos, float ypos){
  /*lineData.screenspaceLines.push_back(
    Line {
      .fromPos = glm::vec3(fromPos.x, fromPos.y, 0.f),
      .toPos = glm::vec3(toPos.x, toPos.y, 0.f),
    }
  );*/
}

void freeLine(LineData& lineData, objid lineId){
  std::vector<PermaLine> newLines;
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
  std::vector<PermaLine> newLines;
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

void drawPermaLines(LineData& lineData, GLint shaderProgram, LineColor color, glm::vec4 tint){
  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(tint));
  if (lineData.permaLines.size() > 0){
    std::vector<Line> lines;
    for (auto permaline : lineData.permaLines){
      if (permaline.color == color){
        lines.push_back(permaline.line);
      }
    }
    drawLines(lines);
  }
}

void drawScreenspaceLines(LineData& lineData, GLint shaderProgram){
  drawLines(lineData.screenspaceLines);
}