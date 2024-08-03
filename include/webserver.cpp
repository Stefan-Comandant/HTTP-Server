#include "webserver.h"
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <winsock2.h>

bool WebServer::file_exists(const std::string file_name,
                            const std::string dir_name) {
  std::string dir = dir_name;
  size_t file_name_pos = file_name.find_last_of("/");

  if (file_name_pos != std::variant_npos) {
    bool names_contain_slashes =
        (dir_name.at(dir_name.size() - 1) == '/' || file_name.at(0) == '/');

    dir +=
        (names_contain_slashes ? "" : "/") + file_name.substr(0, file_name_pos);
  }

  for (std::filesystem::directory_entry p :
       std::filesystem::directory_iterator(dir)) {
    if (p.is_directory()) {
      if (file_exists(file_name, p.path().generic_string())) {
        return true;
      };
      continue;
    }

    size_t pos = p.path().generic_string().find_last_of("/");
    std::string file = p.path().generic_string().substr(pos + 1);

    if (file.compare(std::string(file_name.substr(file_name_pos + 1))) == 0) {
      return true;
    }
  };

  return false;
}

std::vector<char>
WebServer::get_file_contents(const std::string file_path,
                             std::string *content_type,
                             const std::string file_directory) {

  std::string current_absolute_path(MAX_PATH, '\0');

  GetModuleFileName(NULL, current_absolute_path.data(), MAX_PATH);

  std::string file_path_cpy = file_path;

  while (true) {
    size_t pos1 = file_path_cpy.find_first_of('/');
    size_t pos2 = file_path_cpy.find_first_of('\\');

    if (pos1 != 0 && pos2 != 0) {
      break;
    }

    file_path_cpy = file_path_cpy.substr(1);
  }

  std::filesystem::directory_entry entry(current_absolute_path);
  std::filesystem::path file_parent_path =
      entry.path().parent_path().append(file_directory);

  std::filesystem::path full_file_path = file_parent_path;
  full_file_path.append(file_path_cpy);

  std::ifstream file(full_file_path.string(),
                     std::ios_base::binary | std::ios_base::ate);
  if (!file_exists(file_path, file_parent_path.string()) || !file.is_open()) {
    std::cout << "ERROR::Failed to open file\n";
    return {};
  }

  size_t pos = file_path.find_last_of(".");
  std::string extension = file_path.substr(pos);
  if (content_types.find(extension) != content_types.end()) {
    *content_type = content_types.at(extension);
  }

  const std::streamsize file_size = file.tellg();
  file.seekg(0, std::ios_base::beg);
  std::vector<char> buffer(file_size);

  file.read(buffer.data(), file_size);
  file.close();

  return buffer;
};

std::string
WebServer::make_response_body(const WebServer::HTTPCodes status_code,
                              const std::string body,
                              const std::string content_type) {
  std::string response =
      "HTTP/1.1 " + std::to_string(status_code) + " " +
      status_texts.at(status_code) + "\r\n" + "Content-Type: " + content_type +
      "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
  return response;
};

std::string
WebServer::make_response_body(const WebServer::HTTPCodes status_code,
                              const std::vector<char> body,
                              const std::string content_type) {
  std::string response =
      "HTTP/1.1 " + std::to_string(status_code) + " " +
      status_texts.at(status_code) + "\r\n" + "Content-Type: " + content_type +
      "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n";
  response.append(body.begin(), body.end());

  return response;
};

WebServer::Context::Context(SOCKET *sock, Request *request,
                            std::string files_directory,
                            std::map<std::string, std::string> params,
                            std::string response_content_type)
    : sock(sock), request(request), m_file_directory(files_directory),
      m_response_content_type(response_content_type), params(params) {
  this->m_status = OK;
}

WebServer::Context *
WebServer::Context::set_status(const WebServer::HTTPCodes status) {
  this->m_status = status;
  return this;
};

std::string WebServer::Context::param(const std::string name) {
  if (this->params.find(name) == this->params.end()) {
    return "";
  };

  return this->params.at(name);
}

int WebServer::Context::send_string(const std::string str) {
  std::string response = make_response_body(this->m_status, str, "text/plain");

  return send(*this->sock, response.data(), response.size(), 0);
};

void WebServer::Context::send_file(const std::string file_path) {
  std::vector<char> body = get_file_contents(
      file_path, &this->m_response_content_type, this->m_file_directory);

  std::string response =
      make_response_body(this->m_status, body, this->m_response_content_type);
  send(*this->sock, response.data(), response.size(), 0);
};

WebServer::Path::Path(){};

WebServer::Path::Path(std::string method, std::string path,
                      PathHandler *handler, std::regex regex, bool is_dynamic,
                      std::vector<std::string> params)
    : method(method), path(path), main_handler(handler), regex(regex),
      is_dynamic(is_dynamic), params(params){};

bool WebServer::Path::empty() const {
  return (this->path.empty() && this->method.empty() &&
          this->main_handler == nullptr);
};

int WebServer::Router::accept(struct sockaddr_in *addr, int *addrlen) const {
  return ::accept(this->m_sock, (SOCKADDR *)addr, addrlen);
};

void WebServer::Router::handle_request(
    SOCKET sock, const std::unordered_map<std::string, Path> &paths) {
  std::vector<char> buf(40096);

  int error = recv(sock, buf.data(), buf.size(), 0);
  if (error == SOCKET_ERROR) {
    std::cout << "ERROR::recv() failed: " << WSAGetLastError() << '\n';
    closesocket(sock);
    return;
  };

  WebServer::Request request = parse_request(buf.data());

  if (request.method.empty() || request.path.empty() || request.host.empty()) {
    std::string response = RESPONSE_METHOD_NOT_ALLOWED;

    error = send(sock, response.data(), response.size(), 0);
    if (error == SOCKET_ERROR) {
      std::cout << "ERROR::send() failed: " << WSAGetLastError() << '\n';
      closesocket(sock);
    };
    return;
  }

  Path requested_path;
  bool is_matching_path = true;

  if (paths.find(request.path) == paths.end()) {
    size_t dot_pos = request.path.find_last_of('.');
    std::vector<char> body;
    std::string content_type;

    if (dot_pos != std::variant_npos) {
      body = get_file_contents(request.path, &content_type,
                               this->m_file_directory);
    }

    if (!body.empty()) {
      std::string response =
          make_response_body(WebServer::OK, body, content_type);
      send(sock, response.data(), response.size(), 0);
      is_matching_path = true;
    }

    for (std::pair<std::string, Path> pair : paths) {
      if (!pair.second.is_dynamic) {
        continue;
      }

      is_matching_path = std::regex_match(request.path, pair.second.regex);
      if (is_matching_path) {
        requested_path = pair.second;
        break;
      }
    }
  } else {
    requested_path = paths.at(request.path);
  }

  if (!is_matching_path) {
    error = send(sock, RESPONSE_NOT_FOUND.data(), RESPONSE_NOT_FOUND.size(), 0);
    if (error == SOCKET_ERROR) {
      std::cout << "ERROR::send() failed: " << WSAGetLastError() << '\n';
      closesocket(sock);
    };
    return;
  }

  if (request.method.compare(requested_path.method) != 0) {
    error = send(sock, RESPONSE_METHOD_NOT_ALLOWED.data(),
                 RESPONSE_METHOD_NOT_ALLOWED.size(), 0);
    if (error == SOCKET_ERROR) {
      std::cout << "ERROR::send() failed: " << WSAGetLastError() << '\n';
      closesocket(sock);
    };
    return;
  }

  if (!requested_path.is_dynamic) {
    // this->middlewares.find()
    this->execute_middlewares(requested_path.path, false,
                              Context(&sock, &request, this->m_file_directory));

    Context ctx(&sock, &request, this->m_file_directory);
    requested_path.main_handler(ctx);
    return;
  }

  // Parse the params
  std::smatch matches;
  std::regex_match(request.path, matches, requested_path.regex);
  std::map<std::string, std::string> params;

  size_t index = 0;
  size_t offset = 0;

  for (std::smatch::const_iterator match = matches.begin();
       match < matches.end(); match++, index++) {
    if (request.path.compare(match->str()) == 0) {
      offset++;
      continue;
    }

    const std::string key = requested_path.params.at(index - offset);
    params.insert_or_assign(key, match->str());
  }

  this->execute_middlewares(
      requested_path.path, true,
      Context(&sock, &request, this->m_file_directory, params));

  Context ctx(&sock, &request, this->m_file_directory, params);
  requested_path.main_handler(ctx);
}

std::string WebServer::Router::trim(const std::string &str) {
  size_t start = 0, end = str.length();

  // Trim leading whitespace
  while (start < end && std::isspace(str[start])) {
    start++;
  }

  // Trim trailing whitespace
  while (end > start && std::isspace(str[end - 1])) {
    end--;
  }

  return str.substr(start, end - start);
}

WebServer::Request WebServer::Router::parse_request(std::string request) {
  std::istringstream stream(request);
  std::string header;

  Request processed_request;

  bool isFirst = true;
  while (getline(stream, header, '\r')) {
    if (header.empty()) {
      break;
    }

    if (isFirst) {
      std::istringstream requestLine(header);
      requestLine >> processed_request.method >> processed_request.path;
      isFirst = false;
      continue;
    }

    size_t pos = header.find(':');
    if (pos != std::string::npos) {
      std::string key = this->trim(header.substr(0, pos));
      std::string value = this->trim(header.substr(pos + 1));
      if (key == "Host") {
        processed_request.host = value;
        continue;
      }

      processed_request.headers[key] = value;
    }
  }

  std::array<std::string, 5> methods = {"GET", "DELETE", "TRACE", "OPTIONS",
                                        "HEAD"};

  bool is_forbidden_method =
      std::find(std::begin(methods), std::end(methods),
                processed_request.method) != std::end(methods);

  if (is_forbidden_method || processed_request.headers.find("content-length") ==
                                 processed_request.headers.end()) {
    return processed_request;
  }

  size_t body_position = request.size() - 1;
  body_position = request.size() -
                  std::stoi(processed_request.headers.at("content-length"));
  processed_request.body = request.substr(body_position);

  return processed_request;
};

bool WebServer::Router::set_socket_blocking(SOCKET sock, bool blocking) {
  u_long nonblocking_long = blocking ? 0 : 1;
  return (ioctlsocket(sock, FIONBIO, &nonblocking_long) != SOCKET_ERROR);
}

std::regex
WebServer::Router::get_path_regex(const std::string path,
                                  std::vector<std::string> *parameter_names) {
  bool is_valid_path = this->isValidPath(path);
  if (!is_valid_path && path.compare("/") != 0)
    return {};

  // Return back the given path if there is no dynamic segment
  if (path.find_first_of(":") == std::variant_npos) {
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
  std::vector<std::string> parameter_names;

  std::regex regex = this->get_path_regex(path, &parameter_names);

  this->m_paths.insert_or_assign(
      path, Path{method, path, handler, regex, true, parameter_names});
}

void WebServer::Router::execute_middlewares(const std::string path,
                                            bool is_dynamic, Context context) {
  for (std::map<std::string, std::vector<PathHandler *>>::const_iterator it =
           this->m_middlewares.begin();
       it != this->m_middlewares.end(); it++) {

    if (path.find(it->first) != 0) {
      continue;
    }

    for (std::vector<PathHandler *>::const_iterator handler =
             it->second.begin();
         handler != it->second.end(); handler++) {
      (*handler.base())(context);
    }
  }
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

void WebServer::Router::Use(std::vector<PathHandler *> handlers) {
  const std::string path_prefix = "/";

  this->m_middlewares[path_prefix].insert(m_middlewares.at(path_prefix).end(),
                                          handlers.begin(), handlers.begin());
}

void WebServer::Router::Use(const std::string path_prefix,
                            std::vector<PathHandler *> handlers) {
  this->m_middlewares[path_prefix].insert(m_middlewares.at(path_prefix).end(),
                                          handlers.begin(), handlers.end());
}

int WebServer::Router::listen(const int port, const char *address) {
  this->m_addr.sin_family = AF_INET;
  this->m_addr.sin_port = htons(port);
  this->m_addr.sin_addr.s_addr = inet_addr(address);

  int error =
      ::bind(this->m_sock, (SOCKADDR *)&this->m_addr, sizeof(this->m_addr));
  if (error == SOCKET_ERROR) {
    std::cout << "bind error\n";
    return error;
  }

  error = ::listen(this->m_sock, SOMAXCONN);
  if (error == SOCKET_ERROR) {
    std::cout << "listen error\n";
    return error;
  }

  if (!set_socket_blocking(this->m_sock, false)) {
    std::cout << "blocking error\n";
    return SOCKET_ERROR;
  };

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(this->m_sock, &readfds);

  while (true) {
    fd_set reafds_copy = readfds;
    int socket_count = select(0, &reafds_copy, nullptr, nullptr, nullptr);
    if (socket_count == SOCKET_ERROR) {
      std::cout << "ERROR::select() failed: " << WSAGetLastError() << '\n';
      continue;
    }

    for (int i = 0; i < socket_count; i++) {
      SOCKET sock = reafds_copy.fd_array[i];

      if (sock == this->m_sock) {
        SOCKET client = this->accept(nullptr, nullptr);
        if (client == INVALID_SOCKET) {
          std::cout << "ERROR::accept() failed: " << WSAGetLastError() << '\n';
          continue;
        }

        FD_SET(client, &readfds);
      } else {
        this->handle_request(sock, this->m_paths);
        closesocket(sock);
        FD_CLR(sock, &readfds);
      }
    }
  }
};

bool WebServer::Router::isValidPath(const std::string path) {
  // Pattern: /:path/:path/:path....
  std::regex pattern(R"(^\/(?:[^\/:]+|:\w+)(?:\/(?:[^\/:]+|:\w+))*$)");

  return std::regex_match(path, pattern) || path.compare("/") == 0;
}

void WebServer::Router::Get(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "GET");
};
void WebServer::Router::Head(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "HEAD");
};
void WebServer::Router::Post(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "POST");
};
void WebServer::Router::Put(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "PUT");
};
void WebServer::Router::Delete(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "DELETE");
};
void WebServer::Router::Connect(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "CONNECT");
};
void WebServer::Router::Options(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "OPTIONS");
};
void WebServer::Router::Trace(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "TRACE");
};
void WebServer::Router::Patch(const std::string path, PathHandler *handler) {
  this->register_path(path, handler, "PATCH");
};
void WebServer::Router::set_file_source_directory(const std::string dir) {
  this->m_file_directory = dir;
}