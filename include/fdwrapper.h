#ifndef FD_WRAPPER_H
#define FD_WRAPPER_H

#include "webserver.h"

#include <string>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
#endif

struct FD_Listen_Options {
    bool non_blocking = false;
    bool reuse_address = false;
    bool keep_alive = false;
    size_t buffer_size = 4096;
    bool enable_broadcast = false;
    size_t write_buffer_size = 8192;
    size_t read_buffer_size = 8192;
    bool ipv6_support = false;
};

class FD_Wrapper {
    friend WebServer::Router;

private:
    #ifdef _WIN32
        SOCKET fd;
    #else
        int m_fd;
    #endif

    FD_Listen_Options m_options;

    /* 
     * Runs the OS specific initialization.
     * On windows it initializes the Windows Socket API with WSAStartup().
     * On Unix-based systems(Linux for example), there is no need for initialization.
     * */
    void init();
public:
    FD_Wrapper(const FD_Listen_Options options = FD_Listen_Options{});
    ~FD_Wrapper();

    /* Apply the options from the options struct passed in when creating the FD_Wrapper. */
    void apply_options();

    /* 
     * Create the underlying socket structure.
     * On Windows, create a SOCKET type.
     * On Unix-bases systems, creates an int to represent the file descriptor.
     * */
    void socket();

    /*
     * Set the socket to listening mode.
     * Also binds the socket to the specified port and address.
     * */
    void listen(const int port, const std::string ip_address);

    FD_Wrapper accept();
    /* 
     * Wrapper for the read syscall.
     * If successful, returns the number of bytes read.
     * Return value of -1 indicates an arror; errno is set to indicate the specific error.
     * May return less bytes than the length parameter.
     * */
    ssize_t read(char *buffer, size_t length);

    /* 
     * Wrapper for the read syscall.
     * If successful, returns the number of bytes written.
     * Return value of -1 indicates an arror; errno is set to indicate the specific error.
     * May return less bytes than the length parameter;
     * */
    ssize_t write(const char *buffer, size_t length);

    std::vector<FD_Wrapper> select();

    /* Wrapper the close syscall. */
    void close();
};

#endif
