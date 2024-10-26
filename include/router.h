#ifndef ROUTER_H
#define ROUTER_H

#include "webserver.h"
#include "fdwrapper.h"
#include <functional>
#include <unordered_map>

class WebServer::Router{
private:
    FD_Wrapper m_fd;
    HTTP_Request parse_raw_request(const std::string raw_http_request);

    // Map for each registered handler
    // Map the handlers to methods since methods have priority over paths
    // Use HTTP_METHODS enum key for type-safety, instead of strings that aren't guaranted to have the right content
    std::unordered_map<HTTP_METHODS, std::unordered_map<std::string, Path>> paths;

    void register_path(const std::string path, HTTP_METHODS method, Handler handler);

public:
    Router(const FD_Listen_Options options = {});

    void listen(const int port, const std::string address);

    void GET(const std::string path, const Handler handler);
    void HEAD(const std::string path, const Handler handler);
    void POST(const std::string path, const Handler handler);
    void PUT(const std::string path, const Handler handler);
    void DELETE(const std::string path, const Handler handler);
    void CONNECT(const std::string path, const Handler handler);
    void OPTIONS(const std::string path, const Handler handler);
    void TRACE(const std::string path, const Handler handler);
    void PATCH(const std::string path, const Handler handler);
};

#endif
