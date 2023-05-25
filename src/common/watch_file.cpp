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


SingleFileWatch addWatchOnFile(int inotifyFd, std::filesystem::path& filePath){
    int watchDescriptor = inotify_add_watch(inotifyFd, filePath.c_str(), IN_ATTRIB | IN_MODIFY);
    modassert(watchDescriptor != -1, "error adding watch");

    SingleFileWatch singleFileWatch {};
    singleFileWatch.watchDescriptor = watchDescriptor;

    fd_set fds;
    FD_ZERO(&singleFileWatch.fds);
    FD_SET(inotifyFd, &fds);
    int maxFd = inotifyFd;

    singleFileWatch.maxFd = maxFd;
    singleFileWatch.fds = fds;
    singleFileWatch.filepath = std::filesystem::canonical(filePath).string();
    return singleFileWatch;
}


// get all files in directory
// add directory style watch on all directories
// add file system watch on all files
// when directory watch indicates new file, add new watch to that


std::optional<FileWatch> watchFiles(std::string directory, float debouncePeriodSeconds){
    if (directory == ""){
	   return std::nullopt;
    }

    modlog("filewatch", "initialized watch files");
    int inotifyFd = inotify_init();
    modassert(inotifyFd != -1, "failure initializing watch files");

    int inotifyFdDir = inotify_init();

    FileWatch filewatch {
        .inotifyFd = inotifyFd,
        .inotifyFdDir = inotifyFdDir,
    };
    for (auto &filePath : getAllFilesInDirectory(directory)){
        std::cout << "File is: " << std::filesystem::canonical(filePath).string() << std::endl;
        auto filePathName = std::filesystem::canonical(filePath).string();
        modlog("filewatch", std::string("added watch on ") + filePathName);
       
        auto fd = std::filesystem::is_directory(filePath) ? inotifyFdDir : inotifyFd;
        auto singleFileWatch = addWatchOnFile(fd, filePath);
        filewatch.fileWatches[singleFileWatch.watchDescriptor] = singleFileWatch;

    }

    return filewatch;
}

void getChangedFilesFromWatch(std::optional<FileWatch>& filewatchOpt, std::set<std::string>& changedFiles){
    auto filewatch = filewatchOpt.value();
    for (auto &[_, singleFileWatch] : filewatch.fileWatches){
        timeval timeout {
            .tv_sec = 0,
            .tv_usec = 0,
        };
        int numReady = select(singleFileWatch.maxFd + 1, &singleFileWatch.fds, NULL, NULL, &timeout);
        modassert(numReady != -1, "got -1 for select, should look up what this means");

    if (FD_ISSET(filewatch.inotifyFd, &singleFileWatch.fds)) {
            constexpr int bufferSize = 4096;
            char buffer[bufferSize];

            ssize_t bytesRead = read(filewatch.inotifyFd, buffer, bufferSize);
            modassert(bytesRead != -1, "error reading inotify buffer");
   
            char* ptr = buffer;
            while (ptr < buffer + bytesRead) {
                inotify_event* event = reinterpret_cast<inotify_event*>(ptr);
                
                auto filename = filewatch.fileWatches.at(event -> wd).filepath;

                if (event -> wd == singleFileWatch.watchDescriptor) {
                  //changedFiles.insert(std::string("/home/brad/gamedev/mosttrusted/gameresources/build/textures/") + event -> name);
                  //std::cout << "changed 1: " << event -> name << ", wd = " << event -> wd << std::endl;
                  std::cout << "filewatch file changed: " << filename << std::endl;
                }else{
                  std::cout << "filewatch changed 2: " << filename <<  std::endl;
                  changedFiles.insert(filename);
                }
                ptr += sizeof(inotify_event) + event->len;
            }
        }
    }

}

void closeWatch(std::optional<FileWatch> filewatch){
    if (!filewatch.has_value()){
        return;
    }
    for (auto &[_, singleFileWatch] : filewatch.value().fileWatches){
        inotify_rm_watch(filewatch.value().inotifyFd, singleFileWatch.watchDescriptor);
        close(filewatch.value().inotifyFd); 
    }
}

std::set<std::string> pollChangedFiles(std::optional<FileWatch>& filewatch){
    if (!filewatch.has_value()){
        return {};
    }
    std::set<std::string> changedFiles;
    getChangedFilesFromWatch(filewatch, changedFiles);
	return changedFiles;
}