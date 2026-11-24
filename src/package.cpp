#include "./package.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

std::string normalizePath(std::string filename){
   return std::filesystem::path(filename).lexically_normal().generic_string();
}

struct PackageHeader {
   uint32_t packageIdentifier = 109111;
   uint32_t version = 0;
   uint32_t numberOfFiles = 0;
};

struct FileMetadata {
   uint32_t offsetBytes = 0;
   uint32_t sizeBytes = 0;
   uint32_t hashname = 0;
   char name[256] = {};
};

struct Package {
   FILE* handle = NULL;
   PackageHeader header;
   std::vector<FileMetadata> fileMetadata;
};

void guard(size_t value){
	if (value <= 0){
		modassert(false, "guard failure");
	}
}

void packageDirectory(const char* output, std::vector<std::string> dirs){
    FILE* handle = fopen(output, "wb+");
    if (!handle){
	    perror("fopen failed");
    }
    modassert(handle != NULL,  "handle is NULL");

   std::vector<std::string> allFiles;
   for (auto &dir : dirs){
      if (std::filesystem::is_regular_file(dir)){
         allFiles.push_back(dir);
      }else{
         auto files = realfiles::listAllFiles(dir);
         for (auto &file : files){
            allFiles.push_back(file);
         }
      }
   }

	PackageHeader packageHeader {};
   packageHeader.numberOfFiles = allFiles.size();

   // write file header
   guard(fwrite(&packageHeader.packageIdentifier, sizeof(packageHeader.packageIdentifier), 1, handle));
   guard(fwrite(&packageHeader.version, sizeof(packageHeader.version), 1, handle));
   guard(fwrite(&packageHeader.numberOfFiles, sizeof(packageHeader.numberOfFiles), 1, handle));


   uint32_t totalOffset = 0;
   totalOffset += sizeof(packageHeader.packageIdentifier);
   totalOffset += sizeof(packageHeader.version);
   totalOffset += sizeof(packageHeader.numberOfFiles);

   FileMetadata fileMetadata{};
   uint32_t offsetPerMetadata =  sizeof(fileMetadata.offsetBytes) + sizeof(fileMetadata.sizeBytes) + sizeof(fileMetadata.hashname) + 256;
   totalOffset = totalOffset + (offsetPerMetadata * packageHeader.numberOfFiles);

   // write file metadata 
   for (int i = 0; i < packageHeader.numberOfFiles; i++){
      auto fileContent = realfiles::doLoadFile(allFiles.at(i).c_str());

   	FileMetadata fileMetadata {
		  .offsetBytes = totalOffset,
		  .sizeBytes = static_cast<uint32_t>(fileContent.size()),
		  .hashname = 30,
   	};
      totalOffset += fileContent.size();

      modassert(allFiles.at(i).size() <= 255, "file name too big");
 	   std::strncpy(fileMetadata.name, allFiles.at(i).c_str(), sizeof(fileMetadata.name));
   	guard(fwrite(&fileMetadata.offsetBytes, sizeof(fileMetadata.offsetBytes), 1, handle));
   	guard(fwrite(&fileMetadata.sizeBytes, sizeof(fileMetadata.sizeBytes), 1, handle));
   	guard(fwrite(&fileMetadata.hashname, sizeof(fileMetadata.hashname), 1, handle));
   	guard(fwrite(fileMetadata.name, sizeof(char), 256, handle));
   }

   // write file content

   std::cout << "packing files: total = " << allFiles.size() << std::endl;
   for (int i = 0; i < packageHeader.numberOfFiles; i++){
 		auto tempData = realfiles::doLoadFile(allFiles.at(i));	
    	int32_t size = tempData.size();
    	if (size != 0){
    		const char* data = tempData.c_str();
    		guard(fwrite(data, sizeof(char), size, handle));
    		std::cout << "packing file: " << (i + 1) << " / " << allFiles.size() << std::endl;
    	}
   }

   // update the offsets in the header here

   fclose(handle);
   std::cout << "packaged dir: " << print(dirs) << " to " << output << std::endl; 

}

void unpackageDirectory(const char* path, const char* output){

}

Package loadPackage(const char* path){
   auto normalizedPath = normalizePath(path);
   FILE* handle = fopen(normalizedPath.c_str(), "rb");
   if (!handle){
	   perror("fopen failed");
   }
   modassert(handle != NULL,  "handle is NULL");
   Package package {
   	.handle = handle,
   };

	package.header.packageIdentifier = 0;  // just since we want to verify this is actually set
   guard(fread(&package.header.packageIdentifier, sizeof(package.header.packageIdentifier), 1, package.handle));
   guard(fread(&package.header.version, sizeof(package.header.version), 1, package.handle));
   guard(fread(&package.header.numberOfFiles, sizeof(package.header.numberOfFiles), 1, package.handle));

   for (int i = 0; i < package.header.numberOfFiles; i++){
   	package.fileMetadata.push_back(FileMetadata{});
   	FileMetadata& fileMetadata = package.fileMetadata.at(package.fileMetadata.size() - 1);
    	guard(fread(&fileMetadata.offsetBytes, sizeof(fileMetadata.offsetBytes), 1, package.handle));
    	guard(fread(&fileMetadata.sizeBytes, sizeof(fileMetadata.sizeBytes), 1, package.handle));
    	guard(fread(&fileMetadata.hashname, sizeof(fileMetadata.hashname), 1, package.handle));

      // TODO - I need to think about the null terminating of this.  I think the last element handles this since we
      // limit the size as 255 for the string on write and 256 element (index = 255) should always have the 0
    	guard(fread(fileMetadata.name, sizeof(char), 256, package.handle));

    	// 	char name[256];

   }

   return package;
}

void closePackage(Package& package){
	fclose(package.handle);
	package.handle = NULL;
}
std::string readFile(Package& package, const char* file){
   std::string fileStr = normalizePath(file);
   for (auto &fileMetadata : package.fileMetadata){
      std::string name = normalizePath(fileMetadata.name);
      if (name == fileStr){
         std::cout << "found this file!" << name << std::endl;
         char* data = (char*)malloc(fileMetadata.sizeBytes); // give more thought about the size of this
         fseek(package.handle, fileMetadata.offsetBytes, SEEK_SET);
         auto bytesRead = fread(data, sizeof(char), fileMetadata.sizeBytes, package.handle);
         modassert(bytesRead == fileMetadata.sizeBytes, "unexpected read size");

         std::string readData(data, fileMetadata.sizeBytes);
         /*std::string checkData = loadFile(fileStr);
         //std::cout << data << std::endl;
         if (readData == checkData){
            std::cout << "data is the same";
         }else{
            std::cout << "data is not the same" << readData.size() << ", vs " << checkData.size() << std::endl;
         }*/
         free(data);
         return readData;
      }
   }
   modassert(false, std::string("did not find file: ") + std::string(file));
   return "";
}

std::vector<std::string> allFilenames(Package& package){
   std::vector<std::string> files;
   for (auto &fileMetadata : package.fileMetadata){
      files.push_back(fileMetadata.name);
   }
   return files;
}

bool fileExists(Package& package, const char* file){
   std::string fileStr = normalizePath(file);
   for (auto &fileMetadata : package.fileMetadata){
      std::string name = normalizePath(fileMetadata.name);
      if (name == fileStr){
         return true;
      }
   }
   return false;
}

std::optional<Package> mountedPackage;

namespace package_report {

struct PackageFileSize {
   std::string path;
   uintmax_t size;
};

struct PackageSizeNode {
   std::string name;
   std::string path;
   uintmax_t size = 0;
   std::vector<PackageSizeNode> children;
};

std::string htmlEscape(std::string value){
   std::string escaped;
   for (char character : value){
      if (character == '&'){
         escaped += "&amp;";
      }else if (character == '<'){
         escaped += "&lt;";
      }else if (character == '>'){
         escaped += "&gt;";
      }else if (character == '"'){
         escaped += "&quot;";
      }else{
         escaped += character;
      }
   }
   return escaped;
}

std::string jsonEscape(std::string value){
   std::string escaped;
   const char* hex = "0123456789abcdef";
   for (unsigned char character : value){
      if (character == '\\'){
         escaped += "\\\\";
      }else if (character == '"'){
         escaped += "\\\"";
      }else if (character == '\n'){
         escaped += "\\n";
      }else if (character == '\r'){
         escaped += "\\r";
      }else if (character == '\t'){
         escaped += "\\t";
      }else if (character < 0x20 || character == '<' || character == '>' || character == '&'){
         escaped += "\\u00";
         escaped += hex[(character >> 4) & 0x0f];
         escaped += hex[character & 0x0f];
      }else{
         escaped += character;
      }
   }
   return escaped;
}

std::string formatBytes(uintmax_t bytes){
   const char* units[] = { "B", "KiB", "MiB", "GiB" };
   double value = static_cast<double>(bytes);
   size_t unit = 0;
   while (value >= 1024.0 && unit < 3){
      value /= 1024.0;
      unit++;
   }
   std::ostringstream result;
   result << std::fixed << std::setprecision(unit == 0 ? 0 : 2) << value << " " << units[unit];
   return result.str();
}

bool comparePackageFileSize(const PackageFileSize& left, const PackageFileSize& right){
   if (left.size != right.size){
      return left.size > right.size;
   }
   return left.path < right.path;
}

PackageSizeNode* findPackageSizeChild(PackageSizeNode& node, std::string name){
   for (auto& child : node.children){
      if (child.name == name){
         return &child;
      }
   }
   return NULL;
}

void addPackageSizeFile(PackageSizeNode& root, const PackageFileSize& file){
   root.size += file.size;
   PackageSizeNode* current = &root;
   std::stringstream pathStream(file.path);
   std::string part;
   std::string currentPath;
   while (std::getline(pathStream, part, '/')){
      if (part.empty()){
         continue;
      }
      if (!currentPath.empty()){
         currentPath += "/";
      }
      currentPath += part;

      auto child = findPackageSizeChild(*current, part);
      if (child == NULL){
         current->children.push_back(PackageSizeNode {
            .name = part,
            .path = currentPath,
         });
         child = &current->children.back();
      }
      child->size += file.size;
      current = child;
   }
}

bool comparePackageSizeNode(const PackageSizeNode& left, const PackageSizeNode& right){
   if (left.size != right.size){
      return left.size > right.size;
   }
   return left.name < right.name;
}

void sortPackageSizeTree(PackageSizeNode& node){
   std::sort(node.children.begin(), node.children.end(), comparePackageSizeNode);
   for (auto& child : node.children){
      sortPackageSizeTree(child);
   }
}

void writePackageSizeRows(std::ostream& report, const PackageSizeNode& node, size_t parentId, size_t depth, size_t& nextId){
   for (auto& child : node.children){
      auto id = nextId++;
      auto isFolder = !child.children.empty();
      report << "<tr data-id=\"" << id << "\" data-parent=\"" << parentId
             << "\" data-folder=\"" << (isFolder ? "true" : "false") << "\">\n";
      report << "<td style=\"padding-left:" << (depth * 1.25 + 0.5) << "rem\">";
      if (isFolder){
         report << "<button class=\"toggle\" onclick=\"toggleRow(" << id << ", this)\">+</button>";
      }else{
         report << "<span class=\"file-marker\">-</span>";
      }
      report << htmlEscape(child.name) << "</td><td class=\"size\">" << formatBytes(child.size)
             << "</td><td>";
      if (isFolder){
         report << htmlEscape(child.path);
      }else{
         report << "<a class=\"source-link\" data-package-path=\"" << htmlEscape(child.path)
                << "\" onclick=\"openSourceFile(this); return false;\">" << htmlEscape(child.path) << "</a>";
      }
      report << "</td></tr>\n";
      if (isFolder){
         writePackageSizeRows(report, child, id, depth + 1, nextId);
      }
   }
}

bool writePackageSizeReport(const char* output, std::vector<PackageFileSize> files){
   std::sort(files.begin(), files.end(), comparePackageFileSize);

   uintmax_t totalBytes = 0;
   for (auto& file : files){
      totalBytes += file.size;
   }
   PackageSizeNode root { .name = "package", .path = "package" };
   for (auto& file : files){
      addPackageSizeFile(root, file);
   }
   sortPackageSizeTree(root);

   std::ofstream report(output);
   if (!report){
      std::cerr << "unable to write package report: " << output << std::endl;
      return false;
   }

   const uintmax_t packageHeaderBytes = 12;
   const uintmax_t fileMetadataBytes = 268;
   const uintmax_t packageOverhead = packageHeaderBytes + fileMetadataBytes * files.size();

   report << "<!doctype html>\n<html><head><meta charset=\"utf-8\">\n";
   report << "<title>Package size report</title>\n";
   report << "<style>body{font-family:system-ui,sans-serif;margin:2rem;color:#222}"
          << "table{border-collapse:collapse;width:100%;margin-top:1rem}"
          << "th,td{padding:.35rem .5rem;border-bottom:1px solid #ddd;text-align:left}"
          << "th{background:#f3f3f3}.size{text-align:right;white-space:nowrap}"
          << "button.toggle{width:1.5rem;margin-right:.35rem}"
          << ".file-marker{display:inline-block;width:1.85rem;text-align:center;color:#999}"
          << "tr[data-folder=true]{font-weight:600}"
          << ".package-contents tr.stripe{background:#f7f7f7}"
          << ".package-contents tr[data-id]:hover{background:#e8f0f8}"
          << ".source-link{color:inherit;text-decoration:none;cursor:default}"
          << ".source-link.available{color:#06c;text-decoration:underline;cursor:pointer}"
          << ".muted{color:#666}</style></head><body>\n";
   report << "<h1>Package size report</h1>\n";
   report << "<p><strong>" << files.size() << "</strong> files, "
          << "<strong>" << formatBytes(totalBytes) << "</strong> data, "
          << "<strong>" << formatBytes(packageOverhead) << "</strong> estimated metadata overhead.</p>\n";
   report << "<button onclick=\"downloadRawReport()\">Download raw JSON</button>\n";
   report << "<button onclick=\"chooseSourceRoot()\">Choose source root</button> "
          << "<span id=\"source-status\" class=\"muted\">No source directory selected.</span>\n";
   report << "<h2>Package contents</h2>\n";
   report << "<table class=\"package-contents\"><tr><th>Path</th><th class=\"size\">Size</th><th>Total path</th></tr>\n";
   size_t nextId = 1;
   writePackageSizeRows(report, root, 0, 0, nextId);
   report << "</table>\n";
   report << "<script>\n"
          << "const rawReport = {\n"
          << "  fileCount: " << files.size() << ",\n"
          << "  dataBytes: " << totalBytes << ",\n"
          << "  metadataBytes: " << packageOverhead << ",\n"
          << "  files: [\n";
   for (size_t i = 0; i < files.size(); i++){
      auto& file = files.at(i);
      report << "    {path: \"" << jsonEscape(file.path) << "\", sizeBytes: " << file.size << "}";
      if (i + 1 < files.size()){
         report << ",";
      }
      report << "\n";
   }
   report << "  ]\n"
          << "};\n"
          << "function downloadRawReport(){\n"
          << "  const blob = new Blob([JSON.stringify(rawReport, null, 2)], {type: 'application/json'});\n"
          << "  const url = URL.createObjectURL(blob);\n"
          << "  const link = document.createElement('a');\n"
          << "  link.href = url;\n"
          << "  link.download = 'package-size-report.json';\n"
          << "  link.click();\n"
          << "  URL.revokeObjectURL(url);\n"
          << "}\n"
          << "let sourceHandles = new Map();\n"
          << "function normalizeSourcePath(path){\n"
          << "  return path.replace(/\\\\/g, '/').replace(/^((\\.\\/)|(\\.\\.\\/))+/g, '');\n"
          << "}\n"
          << "async function indexSourceDirectory(directory, prefix){\n"
          << "  for await (const entry of directory.values()){\n"
          << "    const path = prefix ? prefix + '/' + entry.name : entry.name;\n"
          << "    if (entry.kind === 'directory') await indexSourceDirectory(entry, path);\n"
          << "    else sourceHandles.set(normalizeSourcePath(path), entry);\n"
          << "  }\n"
          << "}\n"
          << "function findSourceHandle(packagePath){\n"
          << "  const normalized = normalizeSourcePath(packagePath);\n"
          << "  for (const [sourcePath, handle] of sourceHandles){\n"
          << "    if (sourcePath === normalized || sourcePath.endsWith('/' + normalized) || normalized.endsWith('/' + sourcePath)) return handle;\n"
          << "  }\n"
          << "  return null;\n"
          << "}\n"
          << "function updateSourceLinks(){\n"
          << "  document.querySelectorAll('.source-link').forEach(link => {\n"
          << "    link.classList.toggle('available', findSourceHandle(link.dataset.packagePath) !== null);\n"
          << "  });\n"
          << "}\n"
          << "async function chooseSourceRoot(){\n"
          << "  const status = document.getElementById('source-status');\n"
          << "  if (!window.showDirectoryPicker){ status.textContent = 'This browser does not support directory access.'; return; }\n"
          << "  try{\n"
          << "    const directory = await window.showDirectoryPicker();\n"
          << "    sourceHandles = new Map();\n"
          << "    await indexSourceDirectory(directory, '');\n"
          << "    status.textContent = sourceHandles.size + ' source files indexed.';\n"
          << "    updateSourceLinks();\n"
          << "  }catch(error){\n"
          << "    if (error.name !== 'AbortError') status.textContent = 'Unable to access the selected directory.';\n"
          << "  }\n"
          << "}\n"
          << "async function openSourceFile(link){\n"
          << "  const handle = findSourceHandle(link.dataset.packagePath);\n"
          << "  if (!handle){ document.getElementById('source-status').textContent = 'Select the matching source root first.'; return; }\n"
          << "  const file = await handle.getFile();\n"
          << "  const url = URL.createObjectURL(file);\n"
          << "  window.open(url, '_blank', 'noopener');\n"
          << "  setTimeout(() => URL.revokeObjectURL(url), 60000);\n"
          << "}\n"
          << "const rows = Array.from(document.querySelectorAll('tr[data-id]'));\n"
          << "const expanded = new Set();\n"
          << "function updateRows(){\n"
          << "  rows.forEach(row => row.style.display = 'none');\n"
          << "  function showChildren(parent){\n"
          << "    rows.filter(row => row.dataset.parent === String(parent)).forEach(row => {\n"
          << "      row.style.display = '';\n"
          << "      if (expanded.has(Number(row.dataset.id))) showChildren(row.dataset.id);\n"
          << "    });\n"
          << "  }\n"
          << "  showChildren(0);\n"
          << "  rows.filter(row => row.style.display !== 'none').forEach((row, index) => {\n"
          << "    row.classList.toggle('stripe', index % 2 === 1);\n"
          << "  });\n"
          << "}\n"
          << "function toggleRow(id, button){\n"
          << "  if (expanded.has(id)){ expanded.delete(id); button.textContent = '+'; }\n"
          << "  else { expanded.add(id); button.textContent = '-'; }\n"
          << "  updateRows();\n"
          << "}\n"
          << "updateRows();\n"
          << "</script>\n";
   report << "<p class=\"muted\">Folders show aggregate size; expand them to inspect their contents.</p>\n";
   report << "</body></html>\n";
   report.close();

   std::cout << "package size report: " << output << std::endl;
   return true;
}

}

bool packageSizeReport(const char* output){
   modassert(mountedPackage.has_value(), "package report requires a mounted package");

   std::vector<package_report::PackageFileSize> files;
   for (auto& fileMetadata : mountedPackage.value().fileMetadata){
      files.push_back(package_report::PackageFileSize {
         .path = normalizePath(fileMetadata.name),
         .size = fileMetadata.sizeBytes,
      });
   }

   return package_report::writePackageSizeReport(output, files);
}

void unmountPackage(){
   if (mountedPackage.has_value()){
      modlog("package unmount", "success");
      closePackage(mountedPackage.value());
   }
   mountedPackage = std::nullopt;
}
void mountPackage(const char* path){
   if (mountedPackage.has_value()){
      unmountPackage();
   }
   modlog("package mount", path);
   mountedPackage = loadPackage(path);
}


std::string readPackageFile(const char* file){
   return readFile(mountedPackage.value(), file);
}

uint32_t getPackageFileSizeBytes(const char* fileStr){
   auto normalizedPath = normalizePath(fileStr);
   modassert(mountedPackage.has_value(), "package is not mounted");
   for (auto &fileMetadata : mountedPackage.value().fileMetadata){
      std::string name = normalizePath(fileMetadata.name);
      if (name == normalizedPath){
         return fileMetadata.sizeBytes;
      }
   }
   return 0;
}

void printFileInfo(Package& package, const char* file){
   std::string fileStr = normalizePath(file);
   for (auto &fileMetadata : package.fileMetadata){
      std::string name = normalizePath(fileMetadata.name);
      if (name == fileStr){
         std::cout << "package file offsetBytes: " << fileMetadata.offsetBytes << std::endl;
         std::cout << "package file sizeBytes: " << fileMetadata.sizeBytes << std::endl;
         std::cout << "package file hashname: " << fileMetadata.hashname << std::endl;
         std::cout << "package file name: " << fileMetadata.name << std::endl;
      }
   }
}

void loopPackageShell(){
   while(true){
     std::string value;
     std::getline(std::cin, value);
     auto tokens = filterWhitespace(split(value.c_str(), ' '));
     if (tokens.at(0) == "quit"){
      	exit(0);
     }else if (tokens.at(0) == "pack"){
         std::vector<std::string> dirs;
         for (int i = 2; i < tokens.size(); i++){
            dirs.push_back(tokens.at(i));
         }
      	std::cout << "tokens: " << print(tokens) << ", size = " << tokens.size() << std::endl;
      	packageDirectory(tokens.at(1).c_str(), dirs);
     }else if (tokens.at(0) == "unpack"){
         modassert(false, "unpack not yet implemented");
     }else if (tokens.at(0) == "package-info"){
      	auto package = loadPackage(tokens.at(1).c_str());
      	std::cout << "package packageIdentifier: " << package.header.packageIdentifier << std::endl;
      	std::cout << "package version: " << package.header.version << std::endl;
      	std::cout << "package numberOfFiles: " << package.header.numberOfFiles << std::endl;
      	if (tokens.size() >= 3){
      		FileMetadata& fileMetadata = package.fileMetadata.at(std::atoi(tokens.at(2).c_str()));
      		std::cout << "offsetBytes: " << fileMetadata.offsetBytes << std::endl;
      		std::cout << "sizeBytes: " << fileMetadata.sizeBytes << std::endl;
      		std::cout << "hashname: " << fileMetadata.hashname << std::endl;
      		std::cout << "name: " << fileMetadata.name << std::endl;
      	}
     }else if (tokens.at(0) == "mount"){
         mountPackage(tokens.at(1).c_str());
         std::cout << "Mounted package: " << tokens.at(1) << std::endl;
     }else if (tokens.at(0) == "unmount"){
         unmountPackage();
         std::cout << "Unmount success" << std::endl;
     }else if (tokens.at(0) == "cat"){
         if (!mountedPackage.has_value()){
            std::cout << "No package mounted" << std::endl;
            continue;
         }
         if (tokens.size() >= 2){
            std::cout << readFile(mountedPackage.value(), tokens.at(1).c_str()) << std::endl;
         }
     }else if (tokens.at(0) == "file-info"){
         if (!mountedPackage.has_value()){
            std::cout << "No package mounted" << std::endl;
            continue;
         }
         if (tokens.size() >= 2){
            printFileInfo(mountedPackage.value(), tokens.at(1).c_str());
         }
     }else if (tokens.at(0) == "list"){
         if (!mountedPackage.has_value()){
            std::cout << "No package mounted" << std::endl;
            continue;
         }
         auto allFiles = allFilenames(mountedPackage.value());
         std::cout << print(allFiles) << std::endl;
     }else{
      	std::cout << "invalid command got: " << value << std::endl;
      	std::cout << "tokens: " << print(tokens) << ", size = " << tokens.size() << std::endl;
     }
   }		
	
}

////////////

std::string readFileOrPackage(std::string filepath){
   if (!mountedPackage.has_value()){
      return realfiles::doLoadFile(normalizePath(filepath));
   }
   return readPackageFile(filepath.c_str());
}

bool fileExistsFromPackage(std::string filepath){
   if (!mountedPackage.has_value()){
      return realfiles::fileExists(normalizePath(filepath));
   }
   return fileExists(mountedPackage.value(), filepath.c_str());
}

bool directoryExistsFromPackage(std::string filepath){
   if (!mountedPackage.has_value()){
      return std::filesystem::is_directory(normalizePath(filepath));
   }

   auto allFiles = allFilenames(mountedPackage.value());
   for (auto &file : allFiles){
      if (isInFolder(filepath, file, "ModEngine")){
         return true;
      }
   }
   return false;
}


std::vector<std::string> listFilesWithExtensionsFromPackage(std::string folder, std::vector<std::string> extensions){
   if (!mountedPackage.has_value()){
      return realfiles::listFilesWithExtensions(folder, extensions);
   }

   auto allFiles = allFilenames(mountedPackage.value());
   std::vector<std::string> finalFiles;
   for (auto &file : allFiles){
      bool isValidExtension = isExtensionType(file, extensions);
      if (isValidExtension && isInFolder(folder, file, "ModEngine")){ 
         finalFiles.push_back(file);
         std::cout << "this file is of extension: " << print(extensions) << ", file = " << file << std::endl;
      }
   }
   return finalFiles;

}

struct OpenPackageFile {
   unsigned int fileHandle;
   bool nativeFs;
   FILE* nativeFile = NULL;
   bool remove = false;
   size_t packageFileSize = 0;
   size_t packageFileOffset = 0;
   size_t packageCurrentOffset = 0;
};
std::vector<OpenPackageFile> openFiles;
// 
unsigned int openFileOrPackage(std::string filepath){
   static unsigned int fileHandle = 0;
   auto normalizedFilepath = normalizePath(filepath);
   if (mountedPackage.has_value()){
      for (auto &fileMetadata : mountedPackage.value().fileMetadata){
         std::string name = normalizePath(fileMetadata.name);
         if (name == normalizedFilepath){
            size_t size = fileMetadata.sizeBytes;
            size_t offsetBytes = fileMetadata.offsetBytes;
            unsigned int handle = fileHandle;
            openFiles.push_back(OpenPackageFile {
               .fileHandle = handle,
               .nativeFs = false,
               .packageFileSize = size,
               .packageFileOffset = offsetBytes,
            });
            fileHandle++;
            return handle;
         }
      }
      modassert(false, std::string("did not find file: ") + std::string(filepath));
      return 0;
   }

   FILE* file = fopen(normalizedFilepath.c_str(), "rb");
   modassert(file != NULL, "file handle is NULL");
   unsigned int handle = fileHandle;
   openFiles.push_back(OpenPackageFile {
      .fileHandle = handle,
      .nativeFs = true,
      .nativeFile = file,
   });
   fileHandle++;
   return handle;
}
int closeFileOrPackage(unsigned int handle){
   for (auto &file : openFiles){
      if (file.fileHandle == handle){
         if (file.nativeFs){
            auto error = fclose(file.nativeFile);
            modassert(error == 0, "error closing file");
         }
         file.remove = true;
         break;
      }
   }
   std::vector<OpenPackageFile> newFiles;
   for (auto &file : openFiles){
      if (!file.remove){
         newFiles.push_back(file);
      }
   }
   openFiles = newFiles;
   return 0;
}
size_t readFileOrPackage(unsigned int handle, void *ptr, size_t size, size_t nmemb){
   for (auto &file : openFiles){
      if (file.fileHandle == handle){
         if (file.nativeFs){
            return fread(ptr, size, nmemb, file.nativeFile);
         }else{
            if (file.packageCurrentOffset >= file.packageFileSize){
               return 0;
            }
            fseek(mountedPackage.value().handle, file.packageFileOffset + file.packageCurrentOffset, SEEK_SET);
            auto objectsRead = fread(ptr, size, nmemb, mountedPackage.value().handle);
            file.packageCurrentOffset += objectsRead * size;
            return objectsRead;
         }
      }
   }
   modassert(false, "invalid file handle");
   return 0;
}
int seekFileOrPackage(unsigned int handle, int offset, int whence){
   for (auto &file : openFiles){
      if (file.fileHandle == handle){
         if (file.nativeFs){
            return fseek(file.nativeFile, offset, whence);
         }else{
            if (whence == SEEK_SET){
               file.packageCurrentOffset = offset;
            }else if (whence == SEEK_CUR){
               file.packageCurrentOffset += offset;
            }else if (whence == SEEK_END){
               file.packageCurrentOffset = file.packageFileSize + offset;
            }else{
               //modassert(false, "unexpected whence value");
            }
            return 0;
         }
      }
   }
   modassert(false, "invalid file handle");
   return -1;
}
size_t tellFileOrPackage(unsigned int handle){
   for (auto &file : openFiles){
      if (file.fileHandle == handle){
         if (file.nativeFs){
           return ftell(file.nativeFile);
         }else{
           return file.packageCurrentOffset;
         }
      }
   }
   modassert(false, "invalid file handle");
   return 0;
}

size_t getSizeFileOrPackage(unsigned int handle){
    for (auto &file : openFiles){
        if (file.fileHandle == handle){
            if (file.nativeFs){
                long curPos = ftell(file.nativeFile);
                modassert(curPos != -1L, "ftell failed");

                int error = fseek(file.nativeFile, 0, SEEK_END);
                modassert(error == 0, "fseek failed");
                
                long size = ftell(file.nativeFile);
                modassert(size != -1L, "ftell failed");

                error = fseek(file.nativeFile, curPos, SEEK_SET);
                modassert(error == 0, "fseek restore failed");

                return static_cast<size_t>(size);
            }else{
                // package file: size is already tracked
                return file.packageFileSize;
            }
        }
    }
    modassert(false, "invalid file handle");
    return 0;
}