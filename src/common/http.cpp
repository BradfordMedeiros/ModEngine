#include "./http.h"

bool downloadFile(std::string urlStr, std::string outputFile){
    CURL *curl = curl_easy_init();
    if (!curl) {
        modlog("curl init", "init failure");
        return false;
    }

    FILE *fp = fopen(outputFile.c_str(), "wb");
    if (!fp) {
        modlog("curl", "failed opening output file");
        curl_easy_cleanup(curl);
        return false;
    }

    bool success = true;

    curl_easy_setopt(curl, CURLOPT_URL, urlStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        modlog("curl error downloading file", curl_easy_strerror(res));
        success = false;
    }

    fclose(fp);
    curl_easy_cleanup(curl);
    return success;
}

std::optional<std::string> downloadFileInMemory(std::string urlStr, std::string outputFile){
	return std::nullopt;
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

