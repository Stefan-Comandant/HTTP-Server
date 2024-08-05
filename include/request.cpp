#include "libs.h"
#include "webserver.h"
#include <memory>
#include <variant>

int WebServer::Router::accept(struct sockaddr_in *addr, int *addrlen) const {
  return ::accept(this->m_sock, (SOCKADDR *)addr, addrlen);
};

void WebServer::Router::handle_request(SOCKET sock) {
  std::vector<char> buf(40096);

  int error = recv(sock, buf.data(), buf.size(), 0);
  if (error == SOCKET_ERROR) {
    std::cout << "ERROR::recv() failed: " << WSAGetLastError() << '\n';
    closesocket(sock);
    return;
  };

  WebServer::Request request = parse_request(buf.data());

  // static int count = 0;
  // std::ofstream file("req_" + std::to_string(count) + ".http");
  // file << buf.data();
  // file.close();
  // count++;

  if (request.method.empty() || request.path.empty() || request.host.empty()) {
    // std::cout << "Empty!!!!\n";
    error = send(sock, RESPONSE_METHOD_NOT_ALLOWED.data(),
                 RESPONSE_METHOD_NOT_ALLOWED.size(), 0);
    if (error == SOCKET_ERROR) {
      std::cout << "ERROR::send() failed: " << WSAGetLastError() << '\n';
      closesocket(sock);
    };
    return;
  }

  // HTTPCodes err = HTTPCodes::OK;
  std::shared_ptr<HTTPCodes> err = std::make_shared<HTTPCodes>(HTTPCodes::OK);
  Path requested_path =
      get_path(request.path, request.method, this->m_paths, err);

  size_t dot_pos = request.path.find_last_of('.');
  if (*err == HTTPCodes::NotFound && dot_pos != std::variant_npos) {
    std::vector<char> body;
    std::string content_type;

    body =
        get_file_contents(request.path, &content_type, this->m_file_directory);

    if (!body.empty()) {
      std::string response =
          make_response_body(WebServer::OK, body, {}, content_type);
      send(sock, response.data(), response.size(), 0);
      return;
    }
  }

  if (*err == HTTPCodes::NotFound) {
    error = send(sock, RESPONSE_NOT_FOUND.data(), RESPONSE_NOT_FOUND.size(), 0);
    if (error == SOCKET_ERROR) {
      std::cout << "ERROR::send() failed: " << WSAGetLastError() << '\n';
      closesocket(sock);
    };
    return;
  }

  if (*err == HTTPCodes::MethodNotAllowed) {
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
    Context context(&sock, &request, this->m_file_directory);

    bool run_main_handler =
        this->execute_middlewares(requested_path.path, false, context);

    if (!run_main_handler) {
      return;
    }

    Context ctx(&sock, &request, this->m_file_directory);
    context.raw_request = buf.data();
    requested_path.main_handler(&ctx);
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

  Context context(&sock, &request, this->m_file_directory, params);
  context.raw_request = buf.data();

  bool run_main_handler =
      this->execute_middlewares(requested_path.path, true, context);
  if (!run_main_handler) {
    return;
  }

  requested_path.main_handler(&context);
}

std::string WebServer::trim(const std::string &str) {
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

  Request computed_request;

  bool isFirst = true;
  while (std::getline(stream, header, '\r')) {
    if (header.empty()) {
      break;
    }

    if (isFirst) {
      std::istringstream request_line(header);
      request_line >> computed_request.method >> computed_request.path;
      isFirst = false;
      continue;
    }

    size_t pos = header.find(':');
    if (pos == std::string::npos)
      continue;

    std::string key = trim(header.substr(0, pos));
    std::string value = trim(header.substr(pos + 1));

    std::string lower_case_key = key;

    for (char &ch : lower_case_key) {
      ch = tolower(ch);
    }

    if (lower_case_key == "host") {
      computed_request.host = value;
      continue;
    }

    computed_request.headers[key].push_back(value);
  }

  std::array<std::string, 5> methods = {"GET", "DELETE", "TRACE", "OPTIONS",
                                        "HEAD"};

  bool is_forbidden_method =
      std::find(std::begin(methods), std::end(methods),
                computed_request.method) != std::end(methods);

  if (is_forbidden_method || computed_request.headers.find("content-length") ==
                                 computed_request.headers.end()) {
    return computed_request;
  }

  size_t body_position = request.size() - 1;
  size_t content_length;
  for (std::string header : computed_request.headers.at("content-length")) {
    content_length = std::stoi(header);
  }

  body_position = request.size() - content_length;

  computed_request.body = request.substr(body_position);

  return computed_request;
};

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
        this->handle_request(sock);
        closesocket(sock);
        FD_CLR(sock, &readfds);
      }
    }
  }
};