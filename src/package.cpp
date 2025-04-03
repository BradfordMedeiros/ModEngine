#include "./package.h"


void packageDirectory(const char* path, const char* output){
	Package package {
		.header = PackageHeader {
		  .numberOfFiles = 1,
		},
		.fileMetadata = {
			FileMetadata {
				.offsetBytes = 0,
				.sizeBytes = 20,
				.hashname = 30,
				.name = "./testfile",
			},
		},
	};

    FILE* handle = fopen(output, "w+");

    if (!handle){
	    perror("fopen failed");
    }
    modassert(handle != NULL,  "handle is NULL");
    modassert(package.header.numberOfFiles == package.fileMetadata.size(), "number of files is not the same as file metadata size");

    // write file header
    fwrite(&package.header.packageIdentifier, sizeof(package.header.packageIdentifier), 1, handle);
    fwrite(&package.header.version, sizeof(package.header.version), 1, handle);
    fwrite(&package.header.numberOfFiles, sizeof(package.header.numberOfFiles), 1, handle);

    // write file metadata 
    for (int i = 0; i < package.header.numberOfFiles; i++){
    	FileMetadata& fileMetadata = package.fileMetadata.at(i);
    	fwrite(&fileMetadata.offsetBytes, sizeof(fileMetadata.offsetBytes), 1, handle);
    	fwrite(&fileMetadata.sizeBytes, sizeof(fileMetadata.sizeBytes), 1, handle);
    	fwrite(&fileMetadata.hashname, sizeof(fileMetadata.hashname), 1, handle);
    	fwrite(fileMetadata.name, sizeof(char), 256, handle);
    }

    // write file content

    auto allFiles = listAllFiles(path);

    std::cout << "packing files: total = " << allFiles.size() << std::endl;
    for (int i = 0; i < allFiles.size(); i++){
 		auto tempData = loadFile(allFiles.at(i));	
    	int32_t size = tempData.size();
    	const char* data = tempData.c_str();
    	fwrite(data, sizeof(char), size, handle);
    	std::cout << "packing file: " << i << " / " << allFiles.size() << std::endl;
    }

    // update the offsets in the header here

    fclose(handle);
    std::cout << "packaged dir: " << path << " to " << output << std::endl; 

}
void unpackageDirectory(const char* path, const char* output){

}

Package loadPackage(const char* path){

}
void closePackage(Package& package){

}
void readFile(Package& package, const char* _data, int* _sizeBytes){

}

std::vector<std::string> ls(Package& package, const char* path){
    return listFilesAndDir(path);
}
std::string cat(Package& package, const char* path){
   return loadFile(path);	
}

bool mockShell = true;

void loopPackageShell(Package& package){
	if(mockShell){
    	while(true){
    	  std::string value;
    	  std::getline(std::cin, value);
    	  auto tokens = filterWhitespace(split(value.c_str(), ' '));
    	  if (tokens.at(0) == "quit"){
    	  	exit(0);
    	  }else if (tokens.at(0) == "ls"){
    	  	auto paths = ls(package, tokens.size() == 1 ? "." : tokens.at(1).c_str());
    	    for (auto &file : paths){
    	    	std::cout << file << " ";
    	    }
    	    std::cout << std::endl;
    	  }else if (tokens.at(0) == "cat"){
			std::cout << cat(package, tokens.at(1).c_str()) << std::endl;
    	  }else if (tokens.at(0) == "package"){
    	  	std::cout << "tokens: " << print(tokens) << ", size = " << tokens.size() << std::endl;
    	  	packageDirectory(tokens.at(1).c_str(), tokens.at(2).c_str());
    	  }else{
    	  	std::cout << "invalid command got: " << value << std::endl;
    	  	std::cout << "tokens: " << print(tokens) << ", size = " << tokens.size() << std::endl;
    	  }
    	}		
	}else{
		modassert(false, "shell not yet implemented");		
	}
}