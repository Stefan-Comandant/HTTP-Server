#include "libs.h"
#include "webserver.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

WebServer::Cookie::Cookie(WebServer::CookieConfig config) {
  if (config.name.empty() || config.value.empty()) {
    return;
  }

  this->config = config;
}

std::string WebServer::Cookie::String() const {
  std::ostringstream string_stream;

  string_stream << config.name << "=" << config.value << "; ";

  if (config.expires != std::time_t{}) {
    std::tm *gmt = std::gmtime(&config.expires);
    string_stream << std::put_time(gmt, "%a, %d %b %Y %H:%M:%s GMT") << "; ";
  }

  if (config.max_age.count() > 0) {
    string_stream << "Max-Age=" << config.max_age.count() << "; ";
  }

  if (!config.domain.empty()) {
    string_stream << "Domain=" << config.domain << "; ";
  }

  if (!config.path.empty()) {
    string_stream << "Path=" << config.path << ";";
  }

  if (config.secure) {
    string_stream << "Secure; ";
  }

  if (config.HTTP_only) {
    string_stream << "HttpOnly; ";
  }

  if (!config.same_site.empty()) {
    string_stream << "SameSite=" << config.same_site << "; ";
  }

  if (!config.priority.empty()) {
    string_stream << "Priority=" << config.priority;
  }

  return string_stream.str();
};

std::string WebServer::Context::cookies(const std::string name) {
  const std::vector<std::string> cookies = this->request->headers.at("Cookie");

  for (std::string cookie : cookies) {
    std::string pair;
    std::stringstream ss(cookie);

    while (std::getline(ss, pair, ';')) {
      pair = trim(pair);
      const size_t pos = pair.find("=");
      const std::string cookie_name = pair.substr(0, pos);
      const std::string cookie_value = pair.substr(pos + 1);

      if (cookie_name.compare(name) == 0) {
        return cookie_value;
      }
    }
  }

  return "";
}

void WebServer::Context::set_cookie(const CookieConfig cookie) {
  this->response_headers["Set-Cookie"].push_back(Cookie(cookie).String());
}

void WebServer::Context::add_header(const std::string header,
                                    const std::string value) {
  this->response_headers[header].push_back(value);
};