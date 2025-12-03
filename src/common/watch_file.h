#ifndef MOD_WATCH_FILE
#define MOD_WATCH_FILE

#include <filesystem>
#include <string>
#include <functional>
#include <sys/inotify.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include "./util.h"

struct SingleFileWatch {
	std::string filepath;
	int watchDescriptor;
};

struct Watcher {
	int inotifyFd;
	int maxFd;
	fd_set fds;
	std::unordered_map<int, std::string> fileWatches;
};

struct FileWatch {
	std::optional<float> debouncePeriodSeconds;
	Watcher files;
	std::unordered_map<std::string, float> timeFileChanged;
	std::string directory;
};


// 
std::optional<FileWatch> watchFiles(std::string directory, std::optional<float> debouncePeriodSeconds = std::nullopt);
void closeWatch(std::optional<FileWatch> filewatch);
std::set<std::string> pollChangedFiles(std::optional<FileWatch>& filewatch, float currentTime);

#endif 
