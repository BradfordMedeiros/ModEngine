#include "./profiling.h"

void startProfile(std::string description, int id){
  std::cout << "my macro - start profiling" << id << std::endl; 
}
void stopProfile(int id){
  std::cout << "my macro - end profiling" << id<< std::endl; 
}
