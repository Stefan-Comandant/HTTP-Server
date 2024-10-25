#include "../include/http_request.h"
#include "../include/router.h"
#include <algorithm>
#include <sstream>
#include <sys/epoll.h>
#include <variant>

static std::string trim(std::string str){
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char ch){
                return !(std::isspace(ch) || ch == '\r' || ch == '\n');
                }));

    str.erase(std::find_if(str.rbegin(), str.rend(), [](char ch){
                return !(std::isspace(ch) || ch == '\r' || ch == '\n');
                }).base(), str.end());

    return str;
};

WebServer::Router::Router(const FD_Listen_Options options){
    m_fd = FD_Wrapper(options);
    m_fd.socket();
    m_fd.apply_options();
};

void WebServer::Router::listen(const int port, const std::string address){
    m_fd.listen(port, address);
    bool should_close_event_loop = false;

    // TODO: implement a cross-platform solution for the non-blocking event loop
    int epfd = epoll_create(1);

    struct epoll_event ev = {.events = EPOLLIN, .data = {.fd = m_fd.m_fd}};
    int rv = epoll_ctl(epfd, EPOLL_CTL_ADD, m_fd.m_fd, &ev);
    if (rv == -1){
        std::cout << std::string(strerror(errno)) << '\n';
        return;
    }

    const unsigned int MAX_EVENTS_COUNT = 256;

    std::vector<epoll_event> events(MAX_EVENTS_COUNT);
 
    while (!should_close_event_loop){
        const int timeout = -1;
 
        int nfds = epoll_wait(epfd, events.data(), MAX_EVENTS_COUNT, timeout);
        if (nfds == -1){
            perror("Failed to poll for fds");
            return;
        }
 
        for (size_t i = 0; i < nfds; i++){
            struct epoll_event event = events.at(i);
            int fd = event.data.fd;

            if (fd == m_fd.m_fd){
                FD_Wrapper new_client = m_fd.accept();
                struct epoll_event new_ev{
                    .events = EPOLLIN | EPOLLOUT | EPOLLET,
                    .data = epoll_data{
                        .fd = new_client.m_fd,
                    },
                };
                epoll_ctl(epfd, EPOLL_CTL_ADD, new_client.m_fd, &new_ev);
                new_client.m_fd = -1;
                continue;
            }

            FD_Wrapper client;
            client.m_fd = fd;

            const size_t BUFFER_SIZE = 4096;
            std::vector<char> rbuffer(BUFFER_SIZE);
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

            HTTP_Request request = parse_raw_request(rbuffer.data());
            std::string wbuffer = "Hello! Your request has successfully been parsed!";

            bytes_count = client.write(wbuffer.data(), wbuffer.size());

#ifdef _WIN32
            if (bytes_count == SOCKET_ERROR){
                std::cout << "ERROR::Failed to write to client: " << std::to_string(WSAGetLastError()) << std::endl;
                continue;
            }
#else
            if (bytes_count == -1){
                perror("Failed to write to client");
                continue;
            }
#endif

            epoll_ctl(epfd, EPOLL_CTL_DEL, client.m_fd, {});

            client.close();
        }
    }
    close(epfd);
};

WebServer::HTTP_Request WebServer::Router::parse_raw_request(const std::string raw_http_request){
    HTTP_Request request;
    std::istringstream ss(raw_http_request);

    std::string line;

    ss.getline(line.data(), 32, '\r');

    std::istringstream req_line_ss(line.data());

    // Firstly, parse the request line, which contains the method, route and HTTP version 
    req_line_ss >> request.method >> request.path >> request.http_version;

    // Start parsing the headers
    while (std::getline(ss, line, '\r')){
        // Trim line from while-space and \r and \n characters
        line = trim(line);

        size_t colon_char_position = line.find_first_of(':', 0);
        if (colon_char_position == std::variant_npos){
            continue;
        }

        std::string header_key = trim(line.substr(0, colon_char_position));
        std::string header_value = trim(line.substr(colon_char_position + 1));

        // Check if the value for the header was already set, if so, simply append the value to the comma separated list
        try {
            std::string value = request.headers.at(header_key);
            if (value.at(value.size() - 1) == ','){
                value.append(header_value);
            } else {
                value.append(",");
                value.append(header_value);
            }

            request.headers[header_key] = value;
        } catch (std::out_of_range error){
            request.headers[header_key] = header_value;
        };
    }

    // Return early if request doesn't have body
    if (request.method.compare("GET") == 0 || request.method.compare("HEAD") == 0){
        return request;
    }

    try {
        int content_length = std::atoi(request.headers.at("Content-Length").data());
        request.body = trim(raw_http_request.substr(raw_http_request.size() - content_length));
    } catch (std::out_of_range error){
        return request;
    };

    return request;
};
