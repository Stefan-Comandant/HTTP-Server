#include "libs.h"
#include "webserver.h"
#include <algorithm>
#include <vector>

bool WebServer::Router::set_socket_blocking(SOCKET sock, bool blocking) {
  u_long nonblocking_long = blocking ? 0 : 1;
  return (ioctlsocket(sock, FIONBIO, &nonblocking_long) != SOCKET_ERROR);
}

WebServer::Router::Router() {
  WORD wVersionRequested = MAKEWORD(2, 2);

  WSAStartup(wVersionRequested, &this->m_wsadata);

  this->m_sock = socket(AF_INET, SOCK_STREAM, 0);

  if (this->m_sock < 0) {
    std::cout << "ERROR::socket() failed: " << WSAGetLastError() << '\n';
    WSACleanup();
    abort();
  }
};

WebServer::Router::~Router() {
  closesocket(this->m_sock);
  WSACleanup();
};

std::regex
WebServer::Router::get_path_regex(const std::string path,
                                  std::vector<std::string> *parameter_names) {
  bool is_valid_path = this->isValidPath(path);
  if (!is_valid_path && path.compare("/") != 0)
    return {};

  // Return back the given path if there is no dynamic segment
  if (path.find_first_of(":") == std::variant_npos) {
    *parameter_names = {};
    return std::regex(path);
  }

  std::stringstream stream(path);
  std::string route;
  std::string path_cpy = path;

  while (std::getline(stream, route, '/')) {
    if (route.size() <= 0) {
      continue;
    }

    const size_t colon_pos = route.find(':', 0);

    if (colon_pos == std::variant_npos) {
      continue;
    }

    const size_t pos =
        path_cpy.find(route) == std::variant_npos ? -1 : path_cpy.find(route);

    const std::string reg = R"(([A-z0-9]+))";
    path_cpy.erase(pos, route.size());
    path_cpy.insert(pos, reg);

    // Remove the ":" char from the route
    parameter_names->push_back(route.substr(1));
  }

  return std::regex(path_cpy);
};

void WebServer::Router::register_path(const std::string path,
                                      PathHandler *handler,
                                      const std::string method) {
  std::vector<std::string> parameter_names = {};

  std::regex regex = this->get_path_regex(path, &parameter_names);

  std::vector<Path>::const_iterator it =
      std::find_if(this->m_paths.begin(), this->m_paths.end(),
                   [path, method](Path existing_path) -> bool {
                     if (existing_path.method.compare(method) == 0 &&
                         existing_path.path.compare(path) == 0) {
                       return true;
                     }

                     if (existing_path.method.compare(method) == 0 &&
                         existing_path.path.compare(path) != 0) {
                       return false;
                     }

                     if (existing_path.method.compare(method) != 0 &&
                         existing_path.path.compare(path) == 0) {
                       return false;
                     }

                     return false;
                   });

  if (it == this->m_paths.end()) {
    this->m_paths.push_back(
        Path(method, path, handler, regex, parameter_names));
  } else {
    this->m_paths.insert(it,
                         Path(method, path, handler, regex, parameter_names));
  }
}

bool WebServer::Router::isValidPath(const std::string path) {
  // Pattern: /:path/:path/:path....
  std::regex pattern(R"((\/{1,1}([A-Za-z0-9_:=?]+))+\/{0,1})");

  return std::regex_match(path, pattern) || path.compare("/") == 0;
}

void WebServer::Router::Get(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "GET");
};
void WebServer::Router::Head(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "HEAD");
};
void WebServer::Router::Post(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "POST");
};
void WebServer::Router::Put(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "PUT");
};
void WebServer::Router::Delete(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "DELETE");
};
void WebServer::Router::Connect(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "CONNECT");
};
void WebServer::Router::Options(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "OPTIONS");
};
void WebServer::Router::Trace(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "TRACE");
};
void WebServer::Router::Patch(const std::string path, PathHandler handler) {
  this->register_path(path, new PathHandler(handler), "PATCH");
};
void WebServer::Router::set_file_source_directory(const std::string dir) {
  this->m_file_directory = dir;
}