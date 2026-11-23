#pragma once

#include <string>
#include <vector>

struct ResourceDefinition {
  const char* path;
  const char* validationType;
  bool isDirectory;
};

using AdditionalResourceValidator = bool (*)(ResourceDefinition resource);

struct AdditionalResourceValidation {
  std::vector<const char*> additionalPaths;
  std::vector<const char*> additionalDirectories;
  std::vector<std::string> additionalAttributes;
  std::vector<ResourceDefinition> additionalResources;
  AdditionalResourceValidator additionalResourceValidator = nullptr;
};

int validateResourceFiles(std::vector<std::string>& pathsToValidate);
