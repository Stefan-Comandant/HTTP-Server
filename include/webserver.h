#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <functional>
namespace WebServer {

    struct HTTP_Request;

    // Context that will be passed to each path handler.
    class Context;

    // Handler used when registering paths or middlewares with the router's path registering methods (GET/POST/PATCH...).
    typedef std::function<void(Context)> Handler;

    struct Path;

    enum HTTP_METHODS{
        METHOD_INVALID = -1,
        METHOD_GET,
        METHOD_HEAD,
        METHOD_POST,
        METHOD_PUT,
        METHOD_DELETE,
        METHOD_CONNECT,
        METHOD_OPTIONS,
        METHOD_TRACE,
        METHOD_PATCH,
    };

    class Router;
};

#endif
