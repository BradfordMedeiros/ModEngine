#include "./http.h"

// TODO add timeouts and size limits and stuff like that here
// TODO ssl

std::vector<Token> parseFormat(std::string content);

size_t writeCallback(char* data, size_t size, size_t nmemb, void* dataPtr){
  std::string* buffer = static_cast<std::string*>(dataPtr);
  size_t totalSize = size * nmemb;
  buffer -> append(static_cast<char*>(data), totalSize);
  return totalSize;
}
 
bool downloadFileRaw(std::string urlStr, std::optional<std::string> outputFile, std::optional<std::string*> inMemoryBuffer){
	bool inMemory = !outputFile.has_value();
	if (inMemory && !inMemoryBuffer.has_value()){
		modassert(inMemoryBuffer.has_value(), "invalid arguments to downloadFileRaw");
	}
  CURL *curl = curl_easy_init();
  if (!curl) {
    modlog("curl init", "init failure");
    return false;
  }

  bool success = true;
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(curl, CURLOPT_URL, urlStr.c_str());

  FILE* fp = NULL;

  if (!inMemory){
  	fp = fopen(outputFile.value().c_str(), "wb");
  	if (!fp) {
  	  modlog("curl", "failed opening output file");
  	  curl_easy_cleanup(curl);
  	  return false;
  	}
  	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  }else{
  	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)inMemoryBuffer.value());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  }

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    modlog("curl error downloading file", curl_easy_strerror(res));
    success = false;
  }

  if (fp){
		fclose(fp);
  }
  curl_easy_cleanup(curl);
  if (success){
  	modlog("curl", std::string("download success: ") + urlStr);
  }else{
  	modlog("curl", std::string("download failure: ") + urlStr);
  }

  return success;
}


bool downloadFile(std::string urlStr, std::string outputFile){
	return downloadFileRaw(urlStr, outputFile, std::nullopt);
}

std::optional<std::string> downloadFileInMemory(std::string urlStr,  bool* isSuccess){
 	std::string data;
	bool success = downloadFileRaw(urlStr, std::nullopt, &data);
	*isSuccess = success;
	if (success){
		std::cout << "file content: " << data << std::endl;
	}else{
		std::cout << "file content download failure" << std::endl;
	}
	if (!success){
		return std::nullopt;
	}
	return data;
}

bool isServerOnline(std::string url){
    CURL *curl = curl_easy_init();
    if (!curl){
        modlog("curl init", "init failure");
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    bool isOnline = res == CURLE_OK;
    return isOnline;
}



std::optional<ServerConfigProperties> getServerConfig(std::string urlStr,  bool* isSuccess){
  auto fileContent = downloadFileInMemory(urlStr, isSuccess);
  if (!fileContent.has_value()){
    return std::nullopt;
  }

  // TODO I probably shouldn't let a bad config over the network crash the program!
  auto properties = parseFormat(fileContent.value());
  
  return ServerConfigProperties {
    .properties = properties,
  };
}
std::optional<std::string> getProperty(ServerConfigProperties& config, std::string target, std::string attribute){
  for (auto &property : config.properties){
    if (property.target == target && property.attribute == attribute){
      return property.payload;
    }
  };
  return std::nullopt;
}