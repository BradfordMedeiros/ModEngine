#include "./tools.h"

CScriptBinding cscriptCreateToolsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/tools", api);

  return binding;
}
