#pragma once

#include <string>
#include <vector>

struct AdditionalResourceValidation {
  std::vector<const char*> additionalPaths;
  std::vector<const char*> additionalDirectories;
  std::vector<std::string> additionalAttributes;
};

int validateResourceFiles(std::vector<std::string>& pathsToValidate);
