#ifndef PATH_H
#define PATH_H

#include "webserver.h"
#include <string>

struct WebServer::Path {
    HTTP_METHODS method;
    std::string path;
    Handler handler;
};

#endif
