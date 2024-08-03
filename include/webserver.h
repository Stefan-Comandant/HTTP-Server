#include "libs.h"
#include <chrono>
#include <ctime>

#ifndef WEBSERVER_H
#define WEBSERVER_H

const std::string RESPONSE_METHOD_NOT_ALLOWED =
    "HTTP/1.1 405 Method Not Allowed\r\n\r\n";

const std::string RESPONSE_NOT_FOUND = "HTTP/1.1 404 Not found\r\n\r\n";

namespace WebServer {
bool file_exists(const std::string file_name, const std::string dir_name);

enum HTTPCodes {
  // Informational
  Continue = 100,
  SwitchingProtocols = 101,
  Processing = 102,
  EarlyHints = 103,

  // Success
  OK = 200,
  Created = 201,
  Accepted = 202,
  NonAuthoritativeInformation = 203,
  NoContent = 204,
  ResetContent = 205,
  PartialContent = 206,
  MultiStatus = 207,
  AlreadyReported = 208,
  IMUsed = 226,

  // Redirection
  MultipleChoices = 300,
  MovedPermanently = 301,
  Found = 302,
  SeeOther = 303,
  NotModified = 304,
  TemporaryRedirect = 307,
  PermanentRedirect = 308,

  // Client Error
  BadRequest = 400,
  Unauthorized = 401,
  PaymentRequired = 402,
  Forbidden = 403,
  NotFound = 404,
  MethodNotAllowed = 405,
  NotAcceptable = 406,
  ProxyAuthenticationRequired = 407,
  RequestTimeout = 408,
  Conflict = 409,
  Gone = 410,
  LengthRequired = 411,
  PreconditionFailed = 412,
  ContentTooLarge = 413,
  URITooLong = 414,
  UnsupportedMediaType = 415,
  RangeNotSatisfiable = 416,
  ExpectationFailed = 417,
  MisdirectedRequest = 421,
  UnprocessableContent = 422,
  Locked = 423,
  FailedDependency = 424,
  TooEarly = 425,
  UpgradeRequired = 426,
  PreconditionRequired = 428,
  TooManyRequests = 429,
  RequestHeaderFieldsTooLarge = 431,
  UnavailableForLegalReasons = 451,

  // Server Error
  InternalServerError = 500,
  NotImplemented = 501,
  BadGateway = 502,
  ServiceUnavailable = 503,
  GatewayTimeout = 504,
  HTTPVersionNotSupported = 505,
  VariantAlsoNegotiates = 506,
  InsufficientStorage = 507,
  LoopDetected = 508,
  NetworkAuthenticationRequired = 511,
};

static const std::unordered_map<std::string, std::string> content_types = {
    {".html", "text/html"},        {".htm", "text/html"},
    {".css", "text/css"},          {".js", "application/javascript"},
    {".ico", "image/x-icon"},      {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},       {".png", "image/png"},
    {".gif", "image/gif"},         {".svg", "image/svg+xml"},
    {".json", "application/json"}, {".txt", "text/plain"},
    {".xml", "application/xml"},   {".pdf", "application/pdf"},
    {".zip", "application/zip"},   {".csv", "text/csv"},
    {".mp4", "video/mp4"},         {".mp3", "audio/mpeg"},
    {".wav", "audio/wav"},         {".bmp", "image/bmp"},
    {".tiff", "image/tiff"},       {".woff", "font/woff"},
    {".woff2", "font/woff2"},      {".otf", "font/otf"},
    {".ttf", "font/ttf"},
};
static std::map<HTTPCodes, std::string> status_texts{
    {Continue, "Continue"},
    {SwitchingProtocols, "Switching Protocols"},
    {Processing, "Processing"},
    {EarlyHints, "Early Hints"},
    {OK, "OK"},
    {Created, "Created"},
    {Accepted, "Accepted"},
    {NonAuthoritativeInformation, "Non Authoritative Information"},
    {NoContent, "No Content"},
    {ResetContent, "Reset Content"},
    {PartialContent, "Partial Content"},
    {MultiStatus, "Multi Status"},
    {AlreadyReported, "Already Reported"},
    {IMUsed, "IM Used"},
    {MultipleChoices, "Multiple Choices"},
    {MovedPermanently, "Moved Permanently"},
    {Found, "Found"},
    {SeeOther, "See Other"},
    {NotModified, "NotModified"},
    {TemporaryRedirect, "Temporary Redirect"},
    {PermanentRedirect, "Permanent Redirect"},
    {BadRequest, "Bad Request"},
    {Unauthorized, "Unauthorized"},
    {PaymentRequired, "Payment Required"},
    {Forbidden, "Forbidden"},
    {NotFound, "Not Found"},
    {MethodNotAllowed, "Method Not Allowed"},
    {NotAcceptable, "Not Acceptable"},
    {ProxyAuthenticationRequired, "Proxy Authentication Required"},
    {RequestTimeout, "Request Timeout"},
    {Conflict, "Conflict"},
    {Gone, "Gone"},
    {LengthRequired, "Length Required"},
    {PreconditionFailed, "Precondition Failed"},
    {ContentTooLarge, "Content Too Large"},
    {URITooLong, "URI Too Long"},
    {UnsupportedMediaType, "Unsupported Media Type"},
    {RangeNotSatisfiable, "Range Not Satisfiable"},
    {ExpectationFailed, "Expectation Failed"},
    {MisdirectedRequest, "Misdirected Request"},
    {UnprocessableContent, "Unprocessable Content"},
    {Locked, "Locked"},
    {FailedDependency, "Failed Dependency"},
    {TooEarly, "Too Early"},
    {UpgradeRequired, "Upgrade Required"},
    {PreconditionRequired, "Precondition Required"},
    {TooManyRequests, "Too Many Requests"},
    {RequestHeaderFieldsTooLarge, "Request Header Fields Too Large"},
    {UnavailableForLegalReasons, "Unavailable For Legal Reasons"},
    {InternalServerError, "Internal Server Error"},
    {NotImplemented, "Not Implemented"},
    {BadGateway, "Bad Gateway"},
    {ServiceUnavailable, "Service Unavailable"},
    {GatewayTimeout, "Gateway Timeout"},
    {HTTPVersionNotSupported, "HTTP Version Not Supported"},
    {VariantAlsoNegotiates, "Variant Also Negotiates"},
    {InsufficientStorage, "Insufficient Storage"},
    {LoopDetected, "Loop Detected"},
    {NetworkAuthenticationRequired, "Network Authentication Required"},
};

std::vector<char> get_file_contents(const std::string file_path,
                                    std::string *content_type,
                                    const std::string file_directory);

std::string make_response_body(const WebServer::HTTPCodes status_code,
                               const std::string body,
                               const std::string content_type);
std::string make_response_body(const WebServer::HTTPCodes status_code,
                               const std::vector<char> body,
                               const std::string content_type);
struct Request {
public:
  std::string method;
  std::string path;
  std::string host;
  std::map<std::string, std::string> headers;
  std::string body;
};

struct CookieConfig {
  std::string name;
  std::string value;
  std::time_t expires;
  std::chrono::seconds max_age;
  std::string domain;
  std::string path;
  bool secure;
  bool HTTP_only;
  std::string same_site;
  std::string priority;
};

class Cookie {
public:
  CookieConfig config;
  Cookie(CookieConfig config);
  std::string String() const;
};

class Context {
  friend class Router;

private:
  std::map<std::string, std::string> params;
  WebServer::HTTPCodes m_status = WebServer::OK;
  SOCKET *sock;
  std::string m_file_directory;
  std::string m_response_content_type = "";
  bool run_next_handler = true;

public:
  WebServer::Request *request;
  Context(SOCKET *sock, Request *request, std::string files_directory,
          std::map<std::string, std::string> params = {},
          std::string response_content_type = "text/plain");
  void Next();
  Context *set_status(const WebServer::HTTPCodes status);
  std::string param(const std::string name);
  int send_string(const std::string str);
  void send_file(const std::string file_path);
};

typedef void PathHandler(Context *);

struct Path {
public:
  std::string method;
  std::string path;
  std::regex regex;
  bool is_dynamic = false;
  std::vector<std::string> params;
  PathHandler *main_handler;
  Path();
  Path(std::string method, std::string path, PathHandler *handler,
       std::regex regex, bool is_dynamic = false,
       std::vector<std::string> params = {});
  bool empty() const;
};

class Router {
private:
  WSADATA m_wsadata;
  SOCKET m_sock;
  int m_port;
  struct sockaddr_in m_addr;
  std::unordered_map<std::string, Path> m_paths = {};
  std::string m_file_directory;
  std::map<std::string, std::vector<PathHandler *>> m_middlewares;

private:
  int accept(struct sockaddr_in *addr, int *addrlen) const;
  void handle_request(SOCKET sock,
                      const std::unordered_map<std::string, Path> &paths);
  std::string trim(const std::string &str);
  Request parse_request(const std::string request);
  bool set_socket_blocking(SOCKET sock, bool blocking);
  std::regex get_path_regex(const std::string path,
                            std::vector<std::string> *parameter_names);
  void register_path(const std::string path, PathHandler *handler,
                     const std::string method);
  bool execute_middlewares(const std::string path, bool is_dynamic,
                           Context &context);

public:
  Router();
  ~Router();
  void Use(std::vector<PathHandler *> handlers);
  void Use(const std::string path_prefix, std::vector<PathHandler *> handlers);
  int listen(const int port, const char *address);
  bool isValidPath(const std::string path);
  void Get(const std::string path, PathHandler *handler);
  void Head(const std::string path, PathHandler *handler);
  void Post(const std::string path, PathHandler *handler);
  void Put(const std::string path, PathHandler *handler);
  void Delete(const std::string path, PathHandler *handler);
  void Connect(const std::string path, PathHandler *handler);
  void Options(const std::string path, PathHandler *handler);
  void Trace(const std::string path, PathHandler *handler);
  void Patch(const std::string path, PathHandler *handler);
  void set_file_source_directory(const std::string dir);
};
} // namespace WebServer

#endif