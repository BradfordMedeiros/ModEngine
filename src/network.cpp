#include "./network.h"

#define MAX_LISTENING_QUEUE 10
#define NETWORK_BUFFER_SIZE 1024

modsocket createServer(){
	std::cout << "create server placeholder" << std::endl;
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);	
	if (socketFd == 0){
		throw std::runtime_error("error creating socket");
	}

    struct sockaddr_in address = {
    	.sin_family = AF_INET,
    	.sin_port = htons(8000),
    	.sin_addr = in_addr {
    		.s_addr = htonl(INADDR_ANY),
    	},
    };
    
    int enable = 1;
	int reuseAddrValue = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	if (reuseAddrValue < 0){
		throw std::runtime_error("error setting reuseaddr");
	}

	int reusePortValue = setsockopt(socketFd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
	if (reusePortValue < 0){
		throw std::runtime_error("error setting reuseport");
	}

    int bindValue = bind(socketFd, (struct sockaddr*) &address, sizeof(address));
    if (bindValue < 0){
    	throw std::runtime_error("error binding socket");
    }

    int listenValue = listen(socketFd, MAX_LISTENING_QUEUE);
    if (listenValue < 0){
    	throw std::runtime_error("error listening socket");
    }

    struct modsocket socketInfo  = {
        .socketin = address,
        .socketFd = socketFd,
        .maxFd = 1,
    };


    int socketValue = fcntl(socketInfo.socketFd, F_SETFL, fcntl(socketInfo.socketFd, F_GETFL) | O_NONBLOCK);
    if (socketValue < 0){
        std::runtime_error("error setting nonblocking mode for socketin");
    }
 
    return socketInfo;
}


// todo mark accept as non blocking, and then if you got something add it to the queue. 
// then do the select thing, which also is async and then can read it
// https://jameshfisher.com/2017/04/05/set_socket_nonblocking/
// http://beej.us/guide/bgnet/html/multi/selectman.html

bool acceptSocketAndMarkNonBlocking(modsocket& socketInfo){
    int addrlen = sizeof(socketInfo.socketin);

    std::cout << "accepting 0" << std::endl;
    int newSocket =  accept(socketInfo.socketFd, (struct sockaddr *)&socketInfo.socketin,  (socklen_t*)&addrlen); 

    std::cout << "accepting 1" << std::endl;


    if (newSocket < 0){  // @todo what other failure modes here?
        if (errno != EWOULDBLOCK && errno != EAGAIN){
            throw std::runtime_error(std::string("error accepting socket ") + strerror(errno));
        }
    }else{
      std::cout << "accepting 3" << std::endl;
      int value = fcntl(newSocket, F_SETFL, fcntl(newSocket, F_GETFL) | O_NONBLOCK);
      if (value < 0){
        throw std::runtime_error("error setting nonblocking mode for socket");
      }

      std::cout << "setting socket into fds" << std::endl;

      FD_SET(newSocket, &socketInfo.fds);     
      socketInfo.maxFd = newSocket;
      return true;
    }
    return false;
}
void sendDataAndCloseSocket (modsocket& socketInfo, int socketFd){
    FD_CLR(socketFd, &socketInfo.fds);
    char buffer[NETWORK_BUFFER_SIZE];
    std::cout << "starting read" << std::endl;
    int value = read(socketFd, buffer, NETWORK_BUFFER_SIZE);
    std::cout << "ending read" << std::endl;
    const char* okString = "ok";
    send(socketFd, okString, strlen(okString), 0);
    close(socketFd);
}

void getDataFromSocket(modsocket socketInfo){
    while (acceptSocketAndMarkNonBlocking(socketInfo));

    timeval timeout {
        .tv_sec = 0,
        .tv_usec = 0,
    };

    std::cout << "selecting 0" << std::endl;
    int hasData = select(socketInfo.maxFd + 1, &socketInfo.fds, NULL, NULL, &timeout);
    if (hasData <= 0){
        std::cout << "selection: no data" << std::endl;
        if (hasData == -1){
            std::cout << errno << std::endl;
        }
        return;
    }

    std::cout << "selection has data" << std::endl;
    
    std::vector<int> readySocketFds;
    int selectedSocket = -1;
    for (int socketFd = 0; socketFd <= socketInfo.maxFd; socketFd++){
      if (FD_ISSET(socketFd, &socketInfo.fds)){
        readySocketFds.push_back(socketFd);
      }
    }
 
    for(int socketFd:readySocketFds){
        sendDataAndCloseSocket(socketInfo, socketFd);
    }
}

void cleanupSocket(modsocket socketInfo){
    close(socketInfo.socketFd);
}

