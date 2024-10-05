#include "webserver.h"
#include <functional>
#include <string>
#include <unordered_map>

struct WebServer::HTTP_Request{
public:
    std::string method;
    std::string path;
    std::string http_version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};
