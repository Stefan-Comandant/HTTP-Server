#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "webserver.h"
#include <functional>
#include <string>
#include <unordered_map>

struct WebServer::HTTP_Request{
public:
    HTTP_METHODS method;
    std::string path;
    std::string http_version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

#endif
