#include "./symbols.h"

std::unordered_map<std::string, int> goalnameToInt;  // static state
int symbolIndex = -1;                                // static state

int getSymbol(std::string name){
  //std::cout << "get symbol called: " << name << std::endl;
  if (goalnameToInt.find(name) != goalnameToInt.end()){
    return goalnameToInt.at(name);
  }
  symbolIndex++;
  goalnameToInt[name] = symbolIndex;
  return symbolIndex;
} 
std::string nameForSymbol(int symbol){
  for (auto &[name, symbolIndex] : goalnameToInt){
    if (symbolIndex == symbol){
      return name;
    }
  }
  modassert(false, "could not find symbol for: " + symbol);
  return "";
}
