#ifndef MOD_WATCH_FILE
#define MOD_WATCH_FILE

#include <filesystem>
#include <string>
#include <functional>
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>
#include "./util.h"

struct SingleFileWatch {
	std::string filepath;
	fd_set fds;
	int watchDescriptor;
	int maxFd;
};

struct FileWatch {
	int inotifyFd;
	int inotifyFdDir;
	std::unordered_map<int, SingleFileWatch> fileWatches;
};


// 
std::optional<FileWatch> watchFiles(std::string directory, float debouncePeriodSeconds = 1.f);
void closeWatch(std::optional<FileWatch> filewatch);
std::set<std::string> pollChangedFiles(std::optional<FileWatch>& filewatch);

#endif 
