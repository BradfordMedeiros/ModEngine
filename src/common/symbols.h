#ifndef MOD_SYMBOLS
#define MOD_SYMBOLS

#include <string>
#include <unordered_map>

#include "./util.h"  // this just uses modassert, should pull that out into own file or even into here

int getSymbol(std::string name);
std::string nameForSymbol(int symbol);

#endif
