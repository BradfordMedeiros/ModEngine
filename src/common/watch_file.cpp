#include "./watch_file.h"


// verify if directory or just file 

#include <sys/select.h>

void MonitorFileChanges(const std::string& filePath) {

/*
    // Prepare the file descriptor set for select


    while (true) {
        // Wait for events using select
        int numReady = select(maxFd + 1, &fds, nullptr, nullptr, nullptr);
        if (numReady == -1) {
            break;
        }

        if (FD_ISSET(inotifyFd, &fds)) {
            // Read and process inotify events
            constexpr int bufferSize = 4096;
            char buffer[bufferSize];

            ssize_t bytesRead = read(inotifyFd, buffer, bufferSize);
            if (bytesRead == -1) {
                break;
            }

            char* ptr = buffer;
            while (ptr < buffer + bytesRead) {
                inotify_event* event = reinterpret_cast<inotify_event*>(ptr);

                if (event->wd == watchDescriptor && (event->mask & IN_MODIFY)) {
                    std::cout << "File modified: " << event->name << std::endl;
                }

                ptr += sizeof(inotify_event) + event->len;
            }
        }
    }

   ;*/
}

void closeWatch(std::optional<FileWatch> filewatch){
    if (filewatch.has_value()){
        inotify_rm_watch(filewatch.value().inotifyFd, filewatch.value().watchDescriptor);
        close(filewatch.value().inotifyFd);
    }
}

std::optional<FileWatch> watchFiles(std::string directory, float debouncePeriodSeconds){
    if (directory == ""){
	   return std::nullopt;
    }
    std::filesystem::path filePath(directory);
 
    modassert(std::filesystem::exists(filePath), std::string("filesystem: file does not exist: " + directory));
    bool isDirectory = std::filesystem::is_directory(filePath);
    modassert(isDirectory, "filewatch must be directory");
    modlog("file watch", "initialized watch files");

	static int inotifyFd = inotify_init();
	modassert(inotifyFd != -1, "failure initializing watch files");

    int watchDescriptor = inotify_add_watch(inotifyFd, directory.c_str(), IN_ATTRIB | IN_MODIFY);
    modlog("file watch", std::string("added watch on ") + std::filesystem::canonical(filePath).string());
    modassert(watchDescriptor != -1, "error adding watch");

    FileWatch filewatch {};
    filewatch.inotifyFd = inotifyFd;
    filewatch.watchDescriptor = watchDescriptor;

    fd_set fds;
    FD_ZERO(&filewatch.fds);
    FD_SET(inotifyFd, &fds);
    int maxFd = inotifyFd;
    filewatch.maxFd = maxFd;


    while (true) {
        // Wait for events using select
        timeval timeout {
            .tv_sec = 0,
            .tv_usec = 0,
        };
        std::cout << "checking for select events" << std::endl;
        int numReady = select(filewatch.maxFd + 1, &filewatch.fds, NULL, NULL, &timeout);
        modassert(numReady != -1, "got -1 for select, should look up what this means");

        int isSet = FD_ISSET(inotifyFd, &fds);
        std::cout << "is set: " << isSet << std::endl;

        if (isSet) {
            // Read and process inotify events
            constexpr int bufferSize = 4096;
            char buffer[bufferSize];

            ssize_t bytesRead = read(inotifyFd, buffer, bufferSize);
            modassert(bytesRead != -1, "error reading inotify buffer");
   
            char* ptr = buffer;
            while (ptr < buffer + bytesRead) {
                inotify_event* event = reinterpret_cast<inotify_event*>(ptr);

                if (event -> wd == filewatch.watchDescriptor) {
                    std::cout << "File modified: " << event->name << std::endl;
                }

                ptr += sizeof(inotify_event) + event->len;
            }
        }
    }
    closeWatch(filewatch);
    return filewatch;
}



   
std::set<std::string> pollChangedFiles(std::optional<FileWatch>& filewatch){
	return { "one.png" , "one.mp3" };
}