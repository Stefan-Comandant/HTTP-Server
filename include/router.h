#ifndef ROUTER_H
#define ROUTER_H

#include "webserver.h"
#include "path.h"
#include "fdwrapper.h"
#include <functional>
#include <map>
#include <unordered_set>

class WebServer::Router{
private:
    FD_Wrapper m_fd;

    // Map for each registered handler
    // Use HTTP_METHODS enum key for type-safety, instead of strings that aren't guaranted to have the right content
    std::map<const std::string, std::map<HTTP_METHODS, Path>> m_paths;

public:

    static HTTP_Request parse_raw_request(const std::string& raw_http_request);
    
    // Generates an HTTP response in the raw string format.
    // The _status_text_ parameter is optional, if it is not provided the function will use the default status text for each status code(the one mapped to it)
    static std::string generate_raw_http_response(const std::string body, const std::vector<std::string> headers, const std::string http_version, const WebServer::HTTP_CODES status_code, std::string status_text = "");

private:
    void register_path(const std::string path, HTTP_METHODS method, Handler handler);

public:
    Router(const FD_Listen_Options options = {});

    // Set the router file descriptor/socket to listen(with the provided options) for new connections;
    void listen(const int port, const std::string address);

    // Register a handler for the route _path_ with the method GET
    void GET(const std::string path, Handler handler);

    // Register a handler for the route _path_ with the method HEAD
    void HEAD(const std::string path, const Handler handler);

    // Register a handler for the route _path_ with the method POST
    void POST(const std::string path, const Handler handler);

    // Register a handler for the route _path_ with the method PUT
    void PUT(const std::string path, const Handler handler);

    // Register a handler for the route _path_ with the method DELETE
    void DELETE(const std::string path, const Handler handler);

    // Register a handler for the route _path_ with the method CONNECT
    void CONNECT(const std::string path, const Handler handler);

    // Register a handler for the route _path_ with the method OPTIONS
    void OPTIONS(const std::string path, const Handler handler);

    // Register a handler for the route _path_ with the method TRACE
    void TRACE(const std::string path, const Handler handler);

    // Register a handler for the route _path_ with the method PATCh
    void PATCH(const std::string path, const Handler handler);
};

#endif
