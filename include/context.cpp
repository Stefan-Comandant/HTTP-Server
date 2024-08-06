#include "libs.h"
#include "webserver.h"
#include <winsock2.h>

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
  std::string response = make_response_body(
      this->m_status, str, this->response_headers, "text/plain");

  return send(*this->sock, response.data(), response.size(), 0);
};

void WebServer::Context::send_file(const std::string file_path) {
  std::vector<char> body = get_file_contents(
      file_path, &this->m_response_content_type, this->m_file_directory);

  std::string response =
      make_response_body(this->m_status, body, this->response_headers,
                         this->m_response_content_type);
  send(*this->sock, response.data(), response.size(), 0);
};

void WebServer::Context::JSON(nlohmann::json json) {
  std::stringstream ss;
  ss << json;
  this->content_type("application/json");

  std::string response = make_response_body(HTTPCodes::OK, ss.str(), {},
                                            this->m_response_content_type);

  send(*this->sock, response.data(), response.size(), 0);
};

WebServer::Context *WebServer::Context::content_type(std::string content_type) {
  this->m_response_content_type = content_type;
  return this;
}