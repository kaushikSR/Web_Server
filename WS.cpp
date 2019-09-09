
//MULTI-THREADED WEB SERVER using POSIX threads



#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <fstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <iomanip>
#include <pthread.h>
#include <inttypes.h>
#include <unordered_set>
using namespace std;

#ifndef _LOG_H_
#define _LOG_H_

#define LogInfo(x, ...) printf("INFO: " x "\n", ##__VA_ARGS__)
#define LogError(x, ...) printf("ERROR: " x "\n", ##__VA_ARGS__)

#define LOG(msg, ...)
#define LOG_INFO(msg, ...) 
#define LOG_ERROR(msg, ...)

#endif

#define MAX_CONNECTIONS 60000
#include "HTTPParser.h"

string RootPath;

unsigned int Recv_Timeout = 20;                   // Time is in seconds

// Supporting file formats

static unordered_set<string> supported_ft = {
    "jpg", "png", "gif", "pdf", "htm", "html",
};

// sending data 

int Send_Data(int sockfd, const char* buffer, size_t size) {
    size_t Length = size;
    size_t total = 0;
    int c;
    int Packet_Size;
    while(total < size) {
        Packet_Size = (Length < 4096) ? Length : 4096;
        c = send(sockfd, buffer + total, Packet_Size, MSG_NOSIGNAL);
        if (c < 0) return -1;
        total += c;
        Length -= c;
    }
    return 0;
}

#define CHECKZERO(func,Val) \
    if(func==NULL) {                 \
    printf("Error in function  %s line # %d\n",__func__,__LINE__); \
    return Val;                                              \
}

#define ERROR_CHECK(func,Val) \
    if(func) {                     \
    printf("Error in function  %s line # %d\n",__func__,__LINE__); \
    return Val;                                              \
}                                                                  \

// Reading the file from disk

char* Read_from_disk(string filename, unsigned int *fsize) {


    ifstream fptr (filename, ifstream::binary);
    ERROR_CHECK(not fptr.is_open(), NULL);
    fptr.seekg (0, fptr.end);
    *fsize = (unsigned int) fptr.tellg();                                                   // Get the length
    char *newfile = new char[*fsize];                                                       // Allocate memory for file read
    CHECKZERO(newfile,NULL);
    fptr.seekg(0, fptr.beg);
    fptr.read(newfile,*fsize);
    fptr.close();

    return newfile;
}



char * Read_File(const string* fname, unsigned int *fsize){
    string filename = *fname;

    return Read_from_disk(filename, fsize);
}

void file_close(char  *file_ptr){
    delete []file_ptr;
}

void* Req_Handler(void * sock) {

    intptr_t req_socket = (intptr_t) sock;
    char* file_buffer;
    string HttpResp_Hdr;
    string req_file_path, not_found_path;
    HttpResp_Header RespHeader;
    HttpReq_Header ReqHeader;
    LOG_INFO("REQUEST: %ld", req_socket);
    char read_buffer[1024];
    int count;
    int r;

#ifdef HTTP_V11
    timeval recv_timeout;
    recv_timeout.tv_sec = Recv_Timeout; // 10 seconds
    recv_timeout.tv_usec = 0;

    if (setsockopt(req_socket, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout)) < 0){
        cout << "Error: setting options for socket:" << req_socket << ". Error No: " << strerror(errno) << endl;
    }

#endif

    bool wait_next = true;
    while (wait_next) {
        bzero(read_buffer, 1024);
        count = recv(req_socket, read_buffer, 1023, 0);
        if (count < 0) {
            LogError("Receiving request header");
            break;
        } else {
            LOG_INFO("Read bytes: %d", sizeof(read_buffer));
            LOG("%s", read_buffer);

            if (not HttpReq_Parser(&ReqHeader, string(read_buffer))) {
                LogError("Invalid Request header %s", read_buffer);
                break;
            }
            if (ReqHeader.path == "/")
                ReqHeader.path = "/index.html";

            req_file_path = RootPath + ReqHeader.path;
            not_found_path = RootPath + "/Not_Found.html";

            string ext = req_file_path.substr(req_file_path.find_last_of(".") + 1);
            RespHeader.contType = File_Type(ext);

            if (supported_ft.find(ext) == supported_ft.end()) {
                req_file_path = not_found_path;
            }
            
            // READ FILE
            
            unsigned int file_size = 0;
            file_buffer = Read_File(&req_file_path, &file_size);
            if (file_buffer != nullptr) {
                RespHeader.status_code = "200";
                RespHeader.status_string = "OK";
            } else {

            // Status codes on error

		cout << "File not found" << endl;
                file_buffer = Read_File(&not_found_path, &file_size);
                RespHeader.status_code = "404";
                RespHeader.status_string = "Not Found";
                if (file_buffer == nullptr) {
                    char NotFound[] = "<!DOCTYPE html>\n<html><body><h1>Not Found</h1>\n<p>Sorry, the requested path was not found.</p>\n</body></html>\n";
                    file_buffer = NotFound;
                    file_size = sizeof(NotFound);
                }
            }
            
            // BUILD RESPONSE
            
            RespHeader.version = ReqHeader.version;
            RespHeader.contLength = to_string(file_size);
            RespHeader.connection = ReqHeader.connection;
            HttpResp_Hdr = Http_RespBuilder(&RespHeader);
            
            // SEND Header data
            
            r = Send_Data(req_socket, HttpResp_Hdr.c_str(), HttpResp_Hdr.length());
            if (r) {
                LogError("Sending response header");
                break;
            }
            r = Send_Data(req_socket, file_buffer, file_size);
            if (r) {
                LogError("Sending data");
                break;
            }


            if (RespHeader.connection == "close")
                break;
        }
    }
    LOG_INFO("REQ HANDLER %ld THREAD KILLED", Req_Socket);
    close(req_socket);
    pthread_exit(NULL);
}
void * SocketIO (void * port_no){
    int portno = (intptr_t) port_no;
    int Req_Count = 0;
    int sockfd, newsockfd, Failed;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // Create a TCP socket

    sockfd =  socket(AF_INET, SOCK_STREAM, 0);
  
    if (sockfd < 0) {
        cout << "Error: error creating socket for listening" << endl;
        cout << "SOCKETIO THREAD KILLED" << endl;
        pthread_exit(NULL);
    }

    // Setting server's socket address structure
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;                                                          // use IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;                                                  // fill in server IP address
    serv_addr.sin_port = htons(portno);                                                      // convert port number to network byte order and set port

    
    // Bind socket to IP address and port on server
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Error: error binding an IP address and port on the server to our socket" << endl;
        cout << "SOCKETIO THREAD KILLED" << endl;
        pthread_exit(NULL);
    }

    // Listen on the socket for new connections
    listen(sockfd, MAX_CONNECTIONS);

    // The accept() accepts an incoming connection
    clilen = sizeof(cli_addr);


    // accept a connection from a client, returns a new client socket for communication with client
    while (true) {
        
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            LogError("Error creating client socket");
        } else {
            LOG_INFO("New connection from %s on port %" PRId16, 
                    inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
	    //cout << "Allocating pthread memory for new request" << endl;
            pthread_t* tid = (pthread_t*) malloc(sizeof(pthread_t));
            Failed = pthread_create(tid, NULL, Req_Handler, (void *) (intptr_t) newsockfd);
            pthread_detach(*tid);
            free(tid);
            if(Failed)
                LogError("ERROR, in creating request thread. Function exited with %d", Failed);
            Req_Count++;
	    //cout << "Total request count so far " << Req_Count << endl;
        }
        
    }
    LOG_INFO("SOCKETIO THREAD KILLED");
}
int main(int argc, char *argv[]) {
    pthread_t SocketIO_t;
    int port = 9000;
    int Failed;
    if (argc < 5) { 
        LogError("Usage is %s -dir <path> -port <num>", argv[0]);
        exit(EXIT_FAILURE);
    } else { 
        for (int i = 1; i < argc; i=i+2){
            string option = string(argv[i]);
            string nextoption = string(argv[i+1]);
            //cout << option << nextoption << endl;
            if (option == "-dir")
                RootPath = nextoption;
            else {
                if (option == "-port")
                    port = stoi(nextoption,nullptr,10);
            }
        }
        if ((port == 0) || (RootPath.empty())) {
            cout << "Not enough or invalid arguments." << endl;
            exit(EXIT_FAILURE);
        }
    }

    LogInfo("Server started at path %s, port = %d", RootPath.c_str(), port);

    Failed = pthread_create(&SocketIO_t, NULL, SocketIO, (void *) (intptr_t) port);
    if (Failed)
        cout << "ERROR, in creating socketIO thread. Error: " << Failed << endl;
    pthread_exit(NULL);
}
