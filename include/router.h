#include "webserver.h"
#include "fdwrapper.h"

#ifndef ROUTER_H
#define ROUTER_H

class WebServer::Router{
private:
    FD_Wrapper m_fd;
    HTTP_Request parse_raw_request(const std::string raw_http_request);

public:
    Router(const FD_Listen_Options options = {});

    void listen(const int port, const std::string address);
};

#endif
