#ifndef MOD_CSCRIPT_TEST_FEATURES_SELECTION_BINDING
#define MOD_CSCRIPT_TEST_FEATURES_SELECTION_BINDING

#include <vector>
#include <iostream>
#include "../../cscript/cscript_binding.h"

CScriptBinding cscriptCreateSelectionBinding(CustomApiBindings& api);

CScriptBinding cscriptCreateScreenshotBinding(CustomApiBindings& api);

#endif