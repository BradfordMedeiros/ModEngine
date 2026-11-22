#pragma once

#include <string>
#include <vector>

struct AdditionalResourceValidation {
  std::vector<const char*> additionalPaths;
  std::vector<std::string> additionalAttributes;
};

int validateResourceFiles(std::vector<std::string>& pathsToValidate);
