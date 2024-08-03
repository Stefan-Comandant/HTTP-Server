#include "libs.h"
#include "webserver.h"
#include <ctime>
#include <iomanip>
#include <sstream>

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
    string_stream << std::put_time(gmt, "") << "; ";
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