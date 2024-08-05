#include "libs.h"
#include "webserver.h"

bool WebServer::Router::execute_middlewares(const std::string path,
                                            std::shared_ptr<Context> context) {
  context->run_next_handler = true;

  for (std::map<std::string, std::vector<PathHandler *>>::const_iterator it =
           this->m_middlewares.begin();
       it != this->m_middlewares.end(); it++) {

    if (path.find(it->first) != 0) {
      continue;
    }

    for (PathHandler *handler : it->second) {
      context->run_next_handler = false;
      (*handler)(context);
      if (!context->run_next_handler) {
        return false;
      }
    }
  }

  return context->run_next_handler;
}

void WebServer::Router::Use(std::vector<PathHandler *> handlers) {
  const std::string path_prefix = "/";

  this->m_middlewares[path_prefix].insert(m_middlewares.at(path_prefix).end(),
                                          handlers.begin(), handlers.end());
}

void WebServer::Router::Use(const std::string path_prefix,
                            std::vector<PathHandler *> handlers) {
  this->m_middlewares[path_prefix].insert(m_middlewares.at(path_prefix).end(),
                                          handlers.begin(), handlers.end());
}

void WebServer::Context::Next() { this->run_next_handler = true; };
