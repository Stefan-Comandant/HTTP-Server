#include "../include/fdwrapper.h"
#include <sys/socket.h>

WebServer::FD_Wrapper::FD_Wrapper(const FD_Listen_Options options): m_fd(-1), m_options(options){
    init();
};

WebServer::FD_Wrapper::~FD_Wrapper(){
    close();
};

void WebServer::FD_Wrapper::apply_options(){
    if (m_options.reuse_address){
        int opt = 1;
        setsockopt(m_fd, IPPROTO_TCP, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    }

    if (m_options.keep_alive){
        int opt = 1;
        setsockopt(m_fd, IPPROTO_TCP, SO_KEEPALIVE, (char *)&opt, sizeof(opt));

        int idle_time = 5;

#ifdef _WIN32

#else
    setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&idle_time, sizeof(idle_time));

    int interval = 2;
    setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPINTVL, (char*)&interval, sizeof(interval));

    int count = 5;
    setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPCNT, (char*)&count, sizeof(count));

#endif
    }

    if (m_options.non_blocking){
#ifdef _WIN32
        u_long mode = 1;
        if (ioctlsocket(m_fd, FIONBIO, &mode) != 0){
            throw std::runtime_error("Failed to set non-blocking mode: " + std::to_string(WSAGetLastError()));
        }
#else
        int flags = fcntl(m_fd, F_GETFL, 0);
        if (flags == -1){
            throw std::runtime_error("Failed to get flags for fd: " + std::string(strerror(errno)));
        }

        if (fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) == -1){
            throw std::runtime_error("Failed to set non-blocking mode: " + std::string(strerror(errno)));
        }
#endif
    }
};

void WebServer::FD_Wrapper::socket(){
#ifdef _WIN32
    m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET){
        throw std::runtime_error("Failed to create socket.");
    }
#else
    m_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_fd == -1){
        throw std::runtime_error("Failed to create socket.");
    }
#endif
};

void WebServer::FD_Wrapper::listen(const int port, const std::string ip_address){
    if (m_fd == -1){
        throw std::runtime_error("Socket not created.");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (ip_address.compare("0.0.0.0") == 0){
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr.s_addr) <= 0) {
            throw std::runtime_error("Invalid IP address format.");
        }
    }

#ifdef _WIN32
    int rv = ::bind(m_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    if (rv == SOCKET_ERROR){
        throw std::runtime_error("Failed to bind socket.");
    }

    rv = ::listen(m_fd, SOMAXCONN);
    if (rv == SOCKET_ERROR){
        throw std::runtime_error("Failed to listen on socket.");
    }
#else
    int rv = ::bind(m_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    if (rv == -1){
        throw std::runtime_error("Failed to bind socket.");
    }

    rv = ::listen(m_fd, SOMAXCONN);
    if (rv == -1){
        throw std::runtime_error("Failed to listen on socket.");
    }
#endif
};

WebServer::FD_Wrapper WebServer::FD_Wrapper::accept(){
    if (m_fd == -1){
        throw std::runtime_error("Socket not created.");
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

#ifdef _WIN32
    SOCKET new_fd = ::accept(m_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (new_fd == INVALID_SOCKET){
        throw std::runtime_error("Accept error: " + std::to_string(WSAGetLastError()));
    }

    FD_Wrapper new_fd_wrapper;
    new_fd_wrapper.m_fd = new_fd;

    return new_fd_wrapper;
#else
    int new_fd = ::accept(m_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (new_fd == -1){
        throw std::runtime_error("Accept error: " + std::string(strerror(errno)));
    }

    FD_Wrapper new_fd_wrapper;
    new_fd_wrapper.m_fd = new_fd;

    return new_fd_wrapper;
#endif
};

ssize_t WebServer::FD_Wrapper::read(char *buffer, size_t length, int flags){
    if (m_fd == -1){
        throw std::runtime_error("Socket not created.");
    }

    ssize_t rv = ::recv(m_fd, buffer, length, MSG_NOSIGNAL | flags);

    return (rv < 0 ? -1: rv);
};


ssize_t WebServer::FD_Wrapper::write(const char *buffer, size_t length, int flags){
    if (m_fd == -1){
        throw std::runtime_error("Socket not created.");
    }

    ssize_t rv = ::send(m_fd, buffer, length, MSG_NOSIGNAL | flags);

    return (rv < 0 ? -1: rv);

};

void WebServer::FD_Wrapper::init(){
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Failed to initialize winsock.");
    }
#endif
};

void WebServer::FD_Wrapper::close(){
    if (m_fd != -1){
    #ifdef _WIN32
        closesocket(m_fd);
        WSACleanup();
    #else
        ::close(m_fd);
    #endif
        m_fd = -1;
    }
};
