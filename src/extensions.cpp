#include "./extensions.h"

Extensions loadExtensions(std::vector<std::string> extensionFiles){
  std::vector<void*> handles;
  std::vector<func_t> onStart;
  std::vector<func_t> onFrame;
  std::vector<func_t> registerGuileFns;
  std::vector<registerGetModFn> registerGetModuleFns;
  std::vector<registerGetArgFn> registerGetArgsFns;

  std::vector<func_unload> onScriptUnload;

  for (auto extensionFile : extensionFiles){
    void* handle = dlopen(extensionFile.c_str(), RTLD_NOW);

    if (handle == NULL){
      std::cout << dlerror() << std::endl;
      assert(false);
    }
    handles.push_back(handle);

    func_t onStartFn = (func_t)dlsym(handle, "onStart");
    if (onStartFn != NULL){
      onStart.push_back(onStartFn);
    }

    func_t onFrameFn = (func_t)dlsym(handle, "onFrame");
    if(onFrameFn != NULL){
      onFrame.push_back(onFrameFn);
    }

    func_t registerGuileFn = (func_t)dlsym(handle, "registerGuileFns");
    if(registerGuileFn != NULL){
      registerGuileFns.push_back(registerGuileFn);
    }

    registerGetModFn registerGetCurrentModule = (registerGetModFn)dlsym(handle, "registerGetCurrentModule");
    if (registerGetCurrentModule != NULL){
      registerGetModuleFns.push_back(registerGetCurrentModule);
    }

    registerGetArgFn registerGetArgs = (registerGetArgFn)dlsym(handle, "registerGetArgs");
    if (registerGetArgs != NULL){
      registerGetArgsFns.push_back(registerGetArgs);
    }

    func_unload onScriptUnloadFn = (func_unload)dlsym(handle, "onScriptUnload");
    if (onScriptUnloadFn != NULL){
      onScriptUnload.push_back(onScriptUnloadFn);
    } 

  }
  
  Extensions extensions { 
    .handles = handles,
    .onStart = onStart,
    .onFrame = onFrame,
    .registerGuileFns = registerGuileFns,
    .registerGetModuleFns = registerGetModuleFns,
    .registerGetArgsFns = registerGetArgsFns,
    .onScriptUnload = onScriptUnload,
  };
  return extensions;
}

void extensionsInit(Extensions& extensions, func_i getCurrentModule, getArgsFn getArgs){
  for (auto onStart : extensions.onStart){
    onStart();
  }
  for (auto registerGetCurrentModule : extensions.registerGetModuleFns){
    registerGetCurrentModule(getCurrentModule);
  }
  for (auto registerGetArgs : extensions.registerGetArgsFns){
    registerGetArgs(getArgs);
  }
}

void extensionsOnFrame(Extensions& extensions){
  for (auto onFrame : extensions.onFrame){
    onFrame();
  }
}

void unloadExtensions(Extensions& extensions){
  for (auto handle : extensions.handles){
    assert(dlclose(handle) == 0);
  }
}

void registerGuileTypes(Extensions& extensions){
  for (auto handle : extensions.handles){
    func_t registerGuileTypeFn = (func_t)dlsym(handle, "registerGuileTypes");
    if(registerGuileTypeFn != NULL){
      registerGuileTypeFn();   
    }      
  }
}

void extensionsUnloadScript(Extensions& extensions, int32_t scriptId){
  for (auto onScriptUnload : extensions.onScriptUnload){
    onScriptUnload(scriptId);
  }
}