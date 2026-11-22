#include "./resource_validation.h"

#include <iostream>

#include "./package.h"
#include "./resources.h"
#include "./scene/serialization.h"
#include "./common/util.h"

#ifdef ADDITIONAL_SRC_HEADER
  #include STR(ADDITIONAL_SRC_HEADER)
#endif

int validateResourceFiles(std::vector<std::string>& pathsToValidate){
  bool missingFiles = false;
  std::vector<std::string> allFiles;
  for (auto &path : pathsToValidate){
    auto files = listFilesWithExtensionsFromPackage(path, { "rawscene" });
    for (auto &file : files){
      allFiles.push_back(file);
    }
  }

  // I could plugin to the actual data types here and do true validation, but this is pragmatic
  // to find missing files
  for (auto &file : allFiles){
    std::cout << "validating file: " << file << std::endl;
    auto tokens = parseFormat(readFileOrPackage(file));
    for (auto &token : tokens){
      if (token.attribute == "texture"){
        std::cout << "texture: " << token.payload << std::endl;

        if (!fileExistsFromPackage(token.payload)){
          missingFiles = true;
          std::cout << "\033[31mError:  " << token.payload << ", file = " << file << "\033[0m" << std::endl;
        }
      }
      if (token.attribute == "mesh"){
        std::cout << "mesh: " << token.payload << std::endl;
        if (!fileExistsFromPackage(token.payload)){
          missingFiles = true;
          std::cout << "\033[31mError:  " << token.payload << ", file = " << file << "\033[0m" << std::endl;
        }
      }
    }
  }

  for (auto file : resources::coreResources){
    std::cout << "validating file: " << file << std::endl;
    if (!fileExistsFromPackage(file)){
      missingFiles = true;
      std::cout << "\033[31mError:  " << " hardcoded resource core reference missing - file = " << file << "\033[0m" << std::endl;
    }
  }

  #ifdef ADDITIONAL_SRC_HEADER
    for (auto file : getAdditionalPathsToValidate()){
      std::cout << "validating file: " << file << std::endl;
      if (!fileExistsFromPackage(file)){
        missingFiles = true;
        std::cout << "\033[31mError:  " << " hardcoded resource additional reference missing - file = " << file << "\033[0m" << std::endl;
      }
    }
  #endif

  return missingFiles ? 1 : 0;
}
