#ifndef MOD_MAPCOMPILE
#define MOD_MAPCOMPILE

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "./common/util.h"
#include "./scene/common/map.h"

typedef std::function<void(std::string& brushFileOut, MapData& mapData, Entity& entity, bool* shouldWrite, std::vector<GameobjAttributeOpts>& attributes, std::string* modelName)> CompileFn;
typedef std::function<void(MapData& mapData, std::string& generatedScene)> FinalizeFn;
struct CompileMapFns {
  CompileFn compileFn;
  FinalizeFn finalizeFn;
  std::function<std::string(std::string mapFile, std::optional<std::string> templateFile)> getTemplateFn;
};
CompileMapFns getCompileMapForGame();

void setCompileFn(CompileMapFns& compileFns);

#endif