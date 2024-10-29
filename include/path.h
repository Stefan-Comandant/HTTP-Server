#ifndef PATH_H
#define PATH_H

#include "webserver.h"
#include <string>

struct WebServer::Path {
    HTTP_METHODS method;
    std::string path;
    WebServer::Handler main_handler;
};

#endif
