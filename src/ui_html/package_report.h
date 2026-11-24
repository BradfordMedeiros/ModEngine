#ifndef MOD_UI_HTML_PACKAGE_REPORT
#define MOD_UI_HTML_PACKAGE_REPORT

#include "../package.h"

bool packageSizeReport(const char* output);
bool writePackageSizeReport(const char* output, std::vector<PackageFileSize> files);

#endif
