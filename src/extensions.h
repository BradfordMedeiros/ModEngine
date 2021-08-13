#ifndef MOD_EXTENSION
#define MOD_EXTENSION

#include <dlfcn.h>
#include <assert.h>
#include <cstddef>
#include <iostream>
#include <vector>

typedef void (*func_t)();
typedef int32_t (*func_i)();
typedef void (*func_unload)(int32_t);
typedef void (*registerGetModFn)(func_i);

struct Extensions {
  std::vector<void*> handles;
  std::vector<func_t> onStart;
  std::vector<func_t> onFrame;
  std::vector<func_t> registerGuileFns;
  std::vector<registerGetModFn> registerGetModuleFns;
  std::vector<func_unload> onScriptUnload;
};

Extensions loadExtensions(std::vector<std::string> extensionFiles);
void extensionsInit(Extensions& extensions, func_i getCurrentModule);
void extensionsOnFrame(Extensions& extensions);
void unloadExtensions(Extensions& extensions);
void registerGuileTypes(Extensions& extensions);
void extensionsUnloadScript(Extensions& extensions, int32_t scriptId);

#endif
