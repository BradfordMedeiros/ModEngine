#include "./watch_file.h"

std::vector<std::filesystem::path> getAllFilesInDirectory(std::string& directory){
    std::filesystem::path filePath(directory);
    modassert(std::filesystem::exists(filePath), std::string("filesystem: file does not exist: " + directory));
    bool isDirectory = std::filesystem::is_directory(filePath);
    modassert(isDirectory, "filewatch must be directory");
    std::vector<std::filesystem::path> files;
    files.push_back(filePath);
    for (auto &file : std::filesystem::recursive_directory_iterator(filePath)){
        files.push_back(file);
    }
    return files;
}


SingleFileWatch addWatchOnFile(int inotifyFd, std::filesystem::path& filePath, int* maxFd, fd_set* fds, uint32_t mask){
    int watchDescriptor = inotify_add_watch(inotifyFd, filePath.c_str(), mask);
    modassert(watchDescriptor != -1, "error adding watch");

    SingleFileWatch singleFileWatch {};
    singleFileWatch.watchDescriptor = watchDescriptor;

    FD_ZERO(fds);
    FD_SET(inotifyFd, fds);
    *maxFd = inotifyFd;

    singleFileWatch.filepath = std::filesystem::canonical(filePath).string();
    return singleFileWatch;
}


// get all files in directory
// add directory style watch on all directories
// add file system watch on all files
// when directory watch indicates new file, add new watch to that


std::optional<FileWatch> watchFiles(std::string directory, std::optional<float> debouncePeriodSeconds){
    if (directory == ""){
	   return std::nullopt;
    }

    modlog("filewatch", "initialized watch files");

    fd_set fds;
    int inotifyFd = inotify_init();
    modassert(inotifyFd != -1, "failure initializing watch files");

    FileWatch filewatch {
        .debouncePeriodSeconds = debouncePeriodSeconds,
        .files = Watcher {
            .inotifyFd = inotifyFd,
            .maxFd = 0,
            .fds = fds,
        },
        .timeFileChanged = {},
    };
    for (auto &filePath : getAllFilesInDirectory(directory)){
        std::cout << "File is: " << std::filesystem::canonical(filePath).string() << std::endl;
        auto filePathName = std::filesystem::canonical(filePath).string();
        modlog("filewatch", std::string("added watch on ") + filePathName);
       
        auto isDirectory = std::filesystem::is_directory(filePath);
        if (!isDirectory){
          auto singleFileWatch = addWatchOnFile(filewatch.files.inotifyFd, filePath, &filewatch.files.maxFd, &filewatch.files.fds, IN_ATTRIB | IN_MODIFY);
          filewatch.files.fileWatches[singleFileWatch.watchDescriptor] = singleFileWatch.filepath;
        }
    }

    modlog("watcher", std::string("starting watching files, total watches: ") + std::to_string(filewatch.files.fileWatches.size()));
    return filewatch;
}

void getChangedFilesFromWatch(std::optional<FileWatch>& filewatchOpt, std::set<std::string>& changedFiles){
    auto filewatch = filewatchOpt.value();
    timeval timeout {
        .tv_sec = 0,
        .tv_usec = 0,
    };
    int numReady = select(filewatch.files.maxFd + 1, &filewatch.files.fds, NULL, NULL, &timeout);
    modassert(numReady != -1, "got -1 for select, should look up what this means");
    if (FD_ISSET(filewatch.files.inotifyFd, &filewatch.files.fds)) {
        constexpr int bufferSize = 4096;
        char buffer[bufferSize];
        ssize_t bytesRead = read(filewatch.files.inotifyFd, buffer, bufferSize);
        modassert(bytesRead != -1, "error reading inotify buffer");
        char* ptr = buffer;
        while (ptr < buffer + bytesRead) {
            inotify_event* event = reinterpret_cast<inotify_event*>(ptr);
            auto filename = filewatch.files.fileWatches.at(event -> wd);
            changedFiles.insert(filename);
            ptr += sizeof(inotify_event) + event->len;
        }
    }
}


void closeWatch(std::optional<FileWatch> filewatch){
    if (!filewatch.has_value()){
        return;
    }
    for (auto &[watchDescriptor, _] : filewatch.value().files.fileWatches){
        inotify_rm_watch(filewatch.value().files.inotifyFd, watchDescriptor);
        close(filewatch.value().files.inotifyFd); 
    }
}

std::set<std::string> pollChangedFiles(std::optional<FileWatch>& filewatch, float currentTime){
    if (!filewatch.has_value()){
        return {};
    }
    std::set<std::string> changedFiles;
    getChangedFilesFromWatch(filewatch, changedFiles);
    if (!filewatch.value().debouncePeriodSeconds.has_value()){
        return changedFiles;
    }

    for (auto &changedFile : changedFiles){
        filewatch.value().timeFileChanged[changedFile] = currentTime;
    }
    std::set<std::string> debouncedChangedFiles;
    for (auto &[file, lastChangeTime] : filewatch.value().timeFileChanged){
        if ((currentTime - lastChangeTime) >= filewatch.value().debouncePeriodSeconds.value()){
            debouncedChangedFiles.insert(file);
        }
    }
    for (auto &file : debouncedChangedFiles){
        filewatch.value().timeFileChanged.erase(file);
    }
	return debouncedChangedFiles;
}