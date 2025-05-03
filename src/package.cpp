#include "./package.h"

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
      auto files = realfiles::listAllFiles(dir);
      for (auto &file : files){
         allFiles.push_back(file);
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
   FILE* handle = fopen(path, "rb");
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
   std::string fileStr = std::string(file);
   for (auto &fileMetadata : package.fileMetadata){
      std::string name(fileMetadata.name);
      if (name == fileStr){
         std::cout << "found this file!" << name << std::endl;
         char* data = (char*)malloc(fileMetadata.sizeBytes); // give more thought about the size of this
         fseek(package.handle, fileMetadata.offsetBytes, SEEK_SET);
         fread(data, sizeof(char), fileMetadata.sizeBytes, package.handle);

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
   std::string fileStr = std::string(file);
   for (auto &fileMetadata : package.fileMetadata){
      std::string name(fileMetadata.name);
      if (name == fileStr){
         return true;
      }
   }
   return false;
}

std::optional<Package> mountedPackage;
void mountPackage(const char* path){
   if (mountedPackage.has_value()){
      modassert(false, "package is already mounted");
   }
   mountedPackage = loadPackage(path);
}
void unmountPackage(){
   if (mountedPackage.has_value()){
      closePackage(mountedPackage.value());
   }
   mountedPackage = std::nullopt;
}

std::string readPackageFile(const char* file){
   return readFile(mountedPackage.value(), file);
}

uint32_t getPackageFileSizeBytes(const char* fileStr){
   modassert(mountedPackage.has_value(), "package is not mounted");
   for (auto &fileMetadata : mountedPackage.value().fileMetadata){
      std::string name(fileMetadata.name);
      if (name == fileStr){
         return fileMetadata.sizeBytes;
      }
   }
   return 0;
}

void printFileInfo(Package& package, const char* file){
   std::string fileStr = std::string(file);
   for (auto &fileMetadata : package.fileMetadata){
      std::string name(fileMetadata.name);
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
      return realfiles::doLoadFile(filepath);
   }
   return readPackageFile(filepath.c_str());
}

bool fileExistsFromPackage(std::string filepath){
   if (!mountedPackage.has_value()){
      return realfiles::fileExists(filepath);
   }
   return fileExists(mountedPackage.value(), filepath.c_str());
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
   if (mountedPackage.has_value()){
      for (auto &fileMetadata : mountedPackage.value().fileMetadata){
         std::string name(fileMetadata.name);
         if (name == filepath){
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

   FILE* file = fopen(filepath.c_str(), "rb");
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
               modassert(false, "unexpected whence value");
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