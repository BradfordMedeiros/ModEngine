#include "./extensions.h"

Extensions loadExtensions(std::vector<std::string> extensionFiles){
  std::vector<void*> handles;
  std::vector<func_t> onStart;
  std::vector<func_t> onFrame;

  for (auto extensionFile : extensionFiles){
    void* handle = dlopen(extensionFile.c_str(), RTLD_NOW);

    if (handle == NULL){
      std::cout << dlerror() << std::endl;
      assert(false);
    }
    handles.push_back(handle);

    func_t onStartFn = (func_t)dlsym(handle, "onStart");
    assert(onStartFn != NULL);
    onStart.push_back(onStartFn);

    func_t onFrameFn = (func_t)dlsym(handle, "onFrame");
    assert(onFrameFn != NULL);
    onFrame.push_back(onFrameFn);
  }
  
  Extensions extensions { 
    .handles = handles,
    .onStart = onStart,
    .onFrame = onFrame,
  };
  return extensions;
}

void extensionsInit(Extensions& extensions){
  for (auto onStart : extensions.onStart){
    onStart();
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
