#ifndef MOD_EXTENSION
#define MOD_EXTENSION

#include <dlfcn.h>
#include <assert.h>
#include <cstddef>
#include <iostream>
#include <vector>

typedef void (*func_t)();

struct Extensions {
  std::vector<void*> handles;
  std::vector<func_t> onStart;
  std::vector<func_t> onFrame;
};

Extensions loadExtensions(std::vector<std::string> extensionFiles);
void extensionsInit(Extensions& extensions);
void extensionsOnFrame(Extensions& extensions);
void unloadExtensions(Extensions& extensions);

#endif
