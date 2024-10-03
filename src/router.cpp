#include "../include/http_request.h"
#include "../include/router.h"
#include <cstdio>
#include <sstream>
#include <string>

WebServer::Router::Router(const FD_Listen_Options options){
    m_fd = FD_Wrapper(options);
    m_fd.socket();
};

void WebServer::Router::listen(const int port, const std::string address){
    m_fd.listen(port, address);

    bool should_close_event_loop = false;

    while (!should_close_event_loop){
        FD_Wrapper client = m_fd.accept();

        std::string rbuffer(4096, '\0');
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

        HTTP_Request request = parse_raw_request(rbuffer);

        client.close();
    }
};

WebServer::HTTP_Request WebServer::Router::parse_raw_request(const std::string raw_http_request){
    HTTP_Request request;
    std::istringstream ss(raw_http_request);
    
    std::string line;

    // Firstly, parse the request line, which contains the method, route and HTTP version 
    ss.getline(line.data(), 32, '\r');

    std::istringstream req_line_ss(line.data());
    std::cout << req_line_ss.str() << '\n';

    req_line_ss >> request.method >> request.path >> request.http_version;
    std::cout << "Method: " << request.method << std::endl;
    std::cout << "Path: " << request.path << std::endl;
    std::cout << "HTTP Version: " << request.http_version << std::endl;

    return request;
};
