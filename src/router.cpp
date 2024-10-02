#include "../include/router.h"
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>

WebServer::Router::Router(const FD_Listen_Options options){
    m_fd = FD_Wrapper(options);
};

void WebServer::Router::listen(const int port, const std::string address){
    m_fd.listen(port, address);

    bool should_close_event_loop = false;

    while (!should_close_event_loop){
        FD_Wrapper client = m_fd.accept();

        std::vector<char> rbuffer(4096);
        ssize_t bytes_count = client.read(rbuffer.data(), 4096);

#ifdef _WIN32
        if (bytes_count == SOCKET_ERROR){
            std::cout << "ERROR::Failed to read from client: " << std::to_string(WSAGetLastError()) << std::endl;
            continue;
        }
#else
        if (bytes_count == -1){
            perror("Failed to read from client");
            continue;
        }
#endif

        // Parse the raw string into an HTTP request, then respond to client.
    }
};

void WebServer::Router::parse_raw_request(const std::string raw_http_request){};
