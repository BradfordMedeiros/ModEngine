#include "./mapcompile.h"

std::optional<CompileMapFns> compileFns;
void setCompileFn(CompileMapFns& compileFnsVal){
	compileFns = compileFnsVal;
}
CompileMapFns getCompileMapForGame(){
	modassert(compileFns.has_value(), "no compile fns for game provided");
	return compileFns.value();
}

