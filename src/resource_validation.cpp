#include "./resource_validation.h"

#include <iostream>
#include <algorithm>

#include "./package.h"
#include "./resources.h"
#include "./scene/serialization.h"
#include "./common/util.h"

#ifdef ADDITIONAL_SRC_HEADER
  #include STR(ADDITIONAL_SRC_HEADER)
#endif

bool validateResourceReference(std::string path, std::string sourceFile, std::string attribute){
  std::cout << attribute << ": " << path << std::endl;
  if (fileExistsFromPackage(path)){
    return false;
  }

  std::cout << "\033[31mError:  " << path << ", file = " << sourceFile << "\033[0m" << std::endl;
  return true;
}

bool validateCubemapReference(std::string path, std::string sourceFile){
  std::vector<std::string> faces = {
    "right.jpg",
    "left.jpg",
    "top.jpg",
    "bottom.jpg",
    "front.jpg",
    "back.jpg",
  };
  bool missingFiles = false;
  for (auto face : faces){
    missingFiles |= validateResourceReference(path + "/" + face, sourceFile, "cubemap");
  }
  return missingFiles;
}

bool validateShaderReference(std::string shaderPaths, std::string sourceFile){
  bool missingFiles = false;
  for (auto shaderPath : filterWhitespace(split(shaderPaths, ','))){
    missingFiles |= validateResourceReference(shaderPath, sourceFile, "shader");
  }
  return missingFiles;
}

int validateResourceFiles(std::vector<std::string>& pathsToValidate){
  bool missingFiles = false;
  std::vector<std::string> allFiles;
#ifdef ADDITIONAL_SRC_HEADER
  auto additionalValidation = getAdditionalPathsToValidate();
#endif
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
      if (token.attribute == "cubemap"){
        missingFiles |= validateCubemapReference(token.payload, file);
      }else if (token.attribute == "shader"){
        missingFiles |= validateShaderReference(token.payload, file);
      }else if (
        token.attribute == "texture" ||
        token.attribute == "mesh" ||
        token.attribute == "custom-texture" ||
        token.attribute == "opacity-texture" ||
        token.attribute == "normal-texture" ||
        token.attribute == "clip" ||
        token.attribute == "effekseer" ||
        token.attribute == "video" ||
        token.attribute == "map" ||
        token.attribute == "scene" ||
        token.attribute == "script"
      ){
        auto resourcePath = token.payload;
        if (token.attribute == "mesh"){
          auto comma = resourcePath.find(',');
          if (comma != std::string::npos){
            resourcePath = resourcePath.substr(0, comma);
          }
        }
        missingFiles |= validateResourceReference(resourcePath, file, token.attribute);
      }else{
        bool isAdditionalAttribute = false;
        #ifdef ADDITIONAL_SRC_HEADER
          isAdditionalAttribute = std::find(
            additionalValidation.additionalAttributes.begin(),
            additionalValidation.additionalAttributes.end(),
            token.attribute
          ) != additionalValidation.additionalAttributes.end();
        #endif
        if (isAdditionalAttribute){
          missingFiles |= validateResourceReference(token.payload, file, token.attribute);
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
  for (auto directory : resources::coreDirectories){
    std::cout << "validating directory: " << directory << std::endl;
    if (!directoryExistsFromPackage(directory)){
      missingFiles = true;
      std::cout << "\033[31mError:  " << " hardcoded directory core reference missing - file = " << directory << "\033[0m" << std::endl;
    }
  }

  #ifdef ADDITIONAL_SRC_HEADER
    for (auto resource : additionalValidation.additionalResources){
      if (std::string(resource.validationType).empty()){
        continue;
      }
      if (additionalValidation.additionalResourceValidator != nullptr){
        missingFiles |= additionalValidation.additionalResourceValidator(resource);
      }
    }

    for (auto file : additionalValidation.additionalPaths){
      std::cout << "validating file: " << file << std::endl;
      if (!fileExistsFromPackage(file)){
        missingFiles = true;
        std::cout << "\033[31mError:  " << " hardcoded resource additional reference missing - file = " << file << "\033[0m" << std::endl;
      }
    }
    for (auto directory : additionalValidation.additionalDirectories){
      std::cout << "validating directory: " << directory << std::endl;
      if (!directoryExistsFromPackage(directory)){
        missingFiles = true;
        std::cout << "\033[31mError:  " << " hardcoded directory additional reference missing - file = " << directory << "\033[0m" << std::endl;
      }
    }
  #endif

  return missingFiles ? 1 : 0;
}
