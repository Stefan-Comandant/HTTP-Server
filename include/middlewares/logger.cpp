#include "../libs.h"
#include "../webserver.h"

void WebServer::logger_middleware(std::shared_ptr<WebServer::Context> ctx) {
  std::cout << ctx->request->method << " request to " << ctx->request->path
            << std::endl;

  ctx->Next();
};