#include "./package.h"

void guard(size_t value){
	if (value <= 0){
		modassert(false, "guard failure");
	}
}

void packageDirectory(const char* path, const char* output){
    FILE* handle = fopen(output, "wb+");
    if (!handle){
	    perror("fopen failed");
    }
    modassert(handle != NULL,  "handle is NULL");

   auto allFiles = listAllFiles(path);

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
      auto fileContent = loadFile(allFiles.at(i).c_str());

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
 		auto tempData = loadFile(allFiles.at(i));	
    	int32_t size = tempData.size();
    	if (size != 0){
    		const char* data = tempData.c_str();
    		guard(fwrite(data, sizeof(char), size, handle));
    		std::cout << "packing file: " << (i + 1) << " / " << allFiles.size() << std::endl;
    	}
   }

   // update the offsets in the header here

   fclose(handle);
   std::cout << "packaged dir: " << path << " to " << output << std::endl; 

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
    	guard(fread(&fileMetadata.name, sizeof(char), 256, package.handle));

    	// 	char name[256];

   }

   return package;
}
void closePackage(Package& package){
	fclose(package.handle);
	package.handle = NULL;
}
void readFile(Package& package, const char* file){
   std::string fileStr = std::string(file);
   for (auto &fileMetadata : package.fileMetadata){
      std::string name(fileMetadata.name);
      if (name == fileStr){
         std::cout << "found this file!" << std::endl;

         char* data = (char*)malloc(fileMetadata.sizeBytes); // give more thought about the size of this
         fseek(package.handle, fileMetadata.offsetBytes, SEEK_SET);
         fread(data, sizeof(char), fileMetadata.sizeBytes, package.handle);

         std::string readData(data, fileMetadata.sizeBytes);
         std::string checkData = loadFile(fileStr);
         std::cout << data << std::endl;

         if (readData == checkData){
            std::cout << "data is the same";
         }else{
            std::cout << "data is not the same" << readData.size() << ", vs " << checkData.size() << std::endl;
         }
         free(data);
         return;
      }
   }
}


std::vector<std::string> ls(const char* path){
    return listFilesAndDir(path);
}
std::string cat(const char* path){
   return loadFile(path);	
}

bool mockShell = true;

void loopPackageShell(){
	if(mockShell){
    	while(true){
    	  std::string value;
    	  std::getline(std::cin, value);
    	  auto tokens = filterWhitespace(split(value.c_str(), ' '));
    	  if (tokens.at(0) == "quit"){
    	  	exit(0);
    	  }else if (tokens.at(0) == "ls"){
    	  	auto paths = ls(tokens.size() == 1 ? "." : tokens.at(1).c_str());
    	    for (auto &file : paths){
    	    	std::cout << file << " ";
    	    }
    	    std::cout << std::endl;
    	  }else if (tokens.at(0) == "cat"){
			std::cout << cat(tokens.at(1).c_str()) << std::endl;
    	  }else if (tokens.at(0) == "package"){
    	  	std::cout << "tokens: " << print(tokens) << ", size = " << tokens.size() << std::endl;
    	  	packageDirectory(tokens.at(1).c_str(), tokens.at(2).c_str());
    	  }else if (tokens.at(0) == "package-info"){
    	  	auto package = loadPackage(tokens.at(1).c_str());
    	  	std::cout << "package packageIdentifier: " << package.header.packageIdentifier << std::endl;
    	  	std::cout << "package version: " << package.header.version << std::endl;
    	  	std::cout << "package numberOfFiles: " << package.header.numberOfFiles << std::endl;

    	  	if (tokens.size() >= 3){
    	  		FileMetadata& fileMetadata = package.fileMetadata.at(std::atoi(tokens.at(2).c_str()));
    	  		std::cout << "package file offsetBytes: " << fileMetadata.offsetBytes << std::endl;
    	  		std::cout << "package file sizeBytes: " << fileMetadata.sizeBytes << std::endl;
    	  		std::cout << "package file hashname: " << fileMetadata.hashname << std::endl;
    	  		std::cout << "package file name: " << fileMetadata.name << std::endl;
    	  	}

    	  }else if (tokens.at(0) == "read"){
            if (tokens.size() >= 3){
               auto package = loadPackage(tokens.at(1).c_str());
               readFile(package, tokens.at(2).c_str());
            }
        }else{
    	  	std::cout << "invalid command got: " << value << std::endl;
    	  	std::cout << "tokens: " << print(tokens) << ", size = " << tokens.size() << std::endl;
    	  }
    	}		
	}else{
		modassert(false, "shell not yet implemented");		
	}
}