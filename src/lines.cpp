#include "./lines.h"

LineData createLines(){
  return LineData {
    .lineColors = {},
    .shapes = {},
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
  if (color == GREEN){
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, glm::vec4(0.f, 1.f, 0.f, 1.f), textureId, std::nullopt);
  }else if (color == BLUE){
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, glm::vec4(0.f, 0.f, 1.f, 1.f), textureId, std::nullopt);
  }else if (color == RED){
    return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, glm::vec4(1.f, 0.f, 0.f, 1.f), textureId, std::nullopt);
  }
  modassert(false, "invalid color for addLineToNextCycle");
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
void removeTempShapeData(LineData& lineData){
  std::vector<ShapeData> newShapes;
  for (auto &shape : lineData.shapes){
    if (shape.perma){
      newShapes.push_back(shape);
    }
  }
  lineData.shapes.clear();
  lineData.shapes = newShapes;
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

void addShapeData(LineData& lineData, ShapeData shapeData){
  lineData.shapes.push_back(shapeData);
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

bool shaderIsDifferent(std::optional<unsigned int> shader1, std::optional<unsigned int> shader2){
  if (!shader1.has_value() && !shader2.has_value()){
    return false;
  }else if (shader1.has_value() && !shader2.has_value()){
    return true;
  }else if (!shader1.has_value() && shader2.has_value()){
    return true;
  }else if (shader1.value() != shader2.value()){
    return true;
  }
  return false;
}

void drawShapeData(LineData& lineData, unsigned int uiShaderProgram, glm::mat4 ndiOrtho, std::function<FontFamily&(std::optional<std::string>)> fontFamilyByName, std::optional<unsigned int> textureId, unsigned int height, unsigned int width, Mesh& unitXYRect, std::function<std::optional<unsigned int>(std::string&)> getTextureId, bool selectionProgram){
  //std::cout << "text number: " << lineData.text.size() << std::endl;

  std::optional<unsigned int> lastShaderId;
  for (auto &shape : lineData.shapes){
    if (selectionProgram && !shape.selectionId.has_value()){
      continue;
    }
    if (textureIdSame(shape.textureId, textureId)){
      if (shaderIsDifferent(shape.shader, lastShaderId)){
          glUseProgram(uiShaderProgram);
          std::vector<UniformData> uniformData;
          uniformData.push_back(UniformData {
            .name = "projection",
            .value = ndiOrtho,
          });
          uniformData.push_back(UniformData {
            .name = "forceTint",
            .value = false,
          });
          uniformData.push_back(UniformData {
            .name = "textureData",
            .value = Sampler2D {
              .textureUnitId = 0,
            },
          });
          setUniformData(uiShaderProgram, uniformData, { "model", "encodedid2", "tint" });
          glEnable(GL_BLEND);

        if (!shape.shader.has_value()){
          // use default shader
          modlog("load shader", "default shader");
        }else {
          modlog("load shader", std::to_string(shape.shader.value()));
        }
        lastShaderId = shape.shader.value();
      }

      //std::cout << "drawing words: " << text.word << std::endl;
      glUniform1i(glGetUniformLocation(uiShaderProgram, "forceTint"), false);
      glUniform4fv(glGetUniformLocation(uiShaderProgram, "tint"), 1, glm::value_ptr(shape.tint));
      if (shape.selectionId.has_value()){
        //std::cout << "selection id value: " << text.selectionId.value() << std::endl;
        auto id = shape.selectionId.value();
        auto color = getColorFromGameobject(id);
        Color colorTypeColor {
          .r = color.x,
          .g = color.y, 
          .b = color.z,
          .a = color.w,
        };
        auto restoredId = getIdFromColor(colorTypeColor);
        //std::cout << "color is: " << print(colorTypeColor) << " - " << id << " - " << restoredId << std::endl;
        glUniform4fv(glGetUniformLocation(uiShaderProgram, "encodedid"), 1, glm::value_ptr(color));
        glUniform4fv(glGetUniformLocation(uiShaderProgram, "encodedid2"), 1, glm::value_ptr(color));
      }else{
        glUniform4fv(glGetUniformLocation(uiShaderProgram, "encodedid"), 1, glm::value_ptr(getColorFromGameobject(0)));
        glUniform4fv(glGetUniformLocation(uiShaderProgram, "encodedid2"), 1, glm::value_ptr(getColorFromGameobject(0)));
      }

      auto textShapeData = std::get_if<TextShapeData>(&shape.shapeData);
      auto rectShapeData = std::get_if<RectShapeData>(&shape.shapeData);
      auto lineShapeData = std::get_if<LineShapeData>(&shape.shapeData);
      if (textShapeData != NULL){
        modassert(textShapeData, "shape data is not text");
        auto coords = convertTextNDICoords(textShapeData -> left, textShapeData ->  top, height, width, shape.ndi);
        auto adjustedFontSize = convertTextNdiFontsize(height, width, textShapeData -> fontSize, shape.ndi);
        FontFamily& fontFamily = fontFamilyByName(textShapeData -> fontFamily);
        drawWords(uiShaderProgram, fontFamily, textShapeData -> word, coords.x, coords.y, adjustedFontSize, textShapeData -> maxWidthNdi);          
      }else if (rectShapeData != NULL){
        modassert(shape.ndi, "non-ndi rect drawing not supported"); 
        float centerXNdi = rectShapeData -> centerX;
        float centerYNdi = rectShapeData -> centerY;
        float widthNdi = rectShapeData -> width;
        float heightNdi = rectShapeData -> height;

        glm::mat4 scaledAndTranslated = glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(centerXNdi, centerYNdi, 0.f)), glm::vec3(widthNdi, heightNdi, 1.f));
        glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(scaledAndTranslated));
        glUniform1i(glGetUniformLocation(uiShaderProgram, "forceTint"), false);

        unsigned int textureId = -1;
        if (rectShapeData -> texture.has_value()){
          auto texId = getTextureId(rectShapeData -> texture.value());
          if (texId.has_value()){
            textureId = texId.value();
          }
        }
        drawMesh(unitXYRect, uiShaderProgram, textureId);
      }else if (lineShapeData != NULL){
        modassert(shape.ndi, "non-ndi line drawing not supported"); 
        glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));

        glUniform1i(glGetUniformLocation(uiShaderProgram, "forceTint"), true);

        std::vector<Line> lines;
        lines.push_back(Line {
          .fromPos = lineShapeData -> fromPos,
          .toPos = lineShapeData -> toPos,
        });
        //glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(lineByColor.tint));
        drawLines(lines, 1); 
      }
      else {
        modassert(false, "draw shape data type not yet implemented");
      }

    }
  }
}
void disposeTempBufferedData(LineData& lineData){
  removeTempShapeData(lineData);
  removeTempLines(lineData);
}
