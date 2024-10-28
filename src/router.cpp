#include "../include/context.h"
#include "../include/http_request.h"
#include "../include/router.h"
#include "../include/path.h"

#include <algorithm>
#include <cwchar>
#include <sstream>
#include <sys/epoll.h>
#include <variant>

// Trim str string by removing leading and trailing '\r', '\n', ' ' characters.
static std::string trim(std::string str){
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char ch){
                return !(std::isspace(ch) || ch == '\r' || ch == '\n');
                }));

    str.erase(std::find_if(str.rbegin(), str.rend(), [](char ch){
                return !(std::isspace(ch) || ch == '\r' || ch == '\n');
                }).base(), str.end());

    return str;
};

WebServer::Router::Router(const FD_Listen_Options options): m_fd(FD_Wrapper(options)){
    m_fd.socket();
    m_fd.apply_options();
};

void WebServer::Router::listen(const int port, const std::string address){
    m_fd.listen(port, address);
    bool should_close_event_loop = false;

    // TODO: implement a cross-platform solution for the non-blocking event loop
    int epfd = epoll_create1(0);

    struct epoll_event ev = {.events = EPOLLIN, .data = {.fd = m_fd.m_fd}};
    int rv = epoll_ctl(epfd, EPOLL_CTL_ADD, m_fd.m_fd, &ev);
    if (rv == -1){
        std::cout << std::string(strerror(errno)) << '\n';
        return;
    }

    const unsigned int MAX_EVENTS_COUNT = 256;

    std::vector<epoll_event> events(MAX_EVENTS_COUNT);
 
    while (!should_close_event_loop){
        const int timeout = -1;
 
        int nfds = epoll_wait(epfd, events.data(), MAX_EVENTS_COUNT, timeout);
        if (nfds == -1){
            perror("Failed to poll for fds");
            return;
        }
 
        for (size_t i = 0; i < nfds; i++){
            struct epoll_event event = events.at(i);
            int fd = event.data.fd;

            if (fd == m_fd.m_fd){
                FD_Wrapper new_client = m_fd.accept();
                struct epoll_event new_ev{
                    .events = EPOLLIN | EPOLLOUT | EPOLLET,
                    .data = epoll_data{
                        .fd = new_client.m_fd,
                    },
                };
                epoll_ctl(epfd, EPOLL_CTL_ADD, new_client.m_fd, &new_ev);
                new_client.m_fd = -1;
                continue;
            }

            FD_Wrapper client;
            client.m_fd = fd;

            const size_t BUFFER_SIZE = 4096;
            std::vector<char> rbuffer(BUFFER_SIZE);
            ssize_t bytes_count = client.read(rbuffer.data(), 4096);

#ifdef _WIN32
            if (bytes_count == SOCKET_ERROR){
                std::cout << "ERROR::Failed to read from client: " << std::to_string(WSAGetLastError()) << std::endl;
                continue;
            }
#else
            if (bytes_count == -1){
                perror("Failed to read from client");
                continue;
            }
#endif

            HTTP_Request request = parse_raw_request(rbuffer.data());

            bool path_exists = true;
            bool method_allowed = true;

            // Firstly check if the requested path even exists before searching through the nested map
            auto it = m_paths.find(request.path);
            if (it == m_paths.end()){
                // Handle 404 Not found
                path_exists = false;
            }

            std::map<HTTP_METHODS, Path>::const_iterator method_it;

            if (path_exists){
                method_it = it->second.find(request.method);
                if (method_it == it->second.end()) {
                    // Handle 405 Method Not Allowed
                    method_allowed = false;
                } 

            }

            if (path_exists && method_allowed){
                (*method_it).second.main_handler(Context{});
            }


            std::string wbuffer = "Hello! Your request has successfully been parsed!";

            bytes_count = client.write(wbuffer.data(), wbuffer.size());

#ifdef _WIN32
            if (bytes_count == SOCKET_ERROR){
                std::cout << "ERROR::Failed to write to client: " << std::to_string(WSAGetLastError()) << std::endl;
                continue;
            }
#else
            if (bytes_count == -1){
                perror("Failed to write to client");
                continue;
            }
#endif

            epoll_ctl(epfd, EPOLL_CTL_DEL, client.m_fd, {});

            client.close();
        }
    }
    close(epfd);
};

const static std::map<const std::string, WebServer::HTTP_METHODS> HTTP_METHOD_MAP = {
    {"GET", WebServer::METHOD_GET},
    {"HEAD", WebServer::METHOD_HEAD},
    {"POST", WebServer::METHOD_POST},
    {"PUT", WebServer::METHOD_PUT},
    {"DELETE", WebServer::METHOD_DELETE},
    {"CONNECT", WebServer::METHOD_CONNECT},
    {"OPTIONS", WebServer::METHOD_OPTIONS},
    {"TRACE", WebServer::METHOD_TRACE},
    {"PATCH", WebServer::METHOD_PATCH},
};

const std::map<WebServer::HTTP_CODES, std::string> STATUS_TEXTS_MAP {
    {WebServer::HTTP_CODES::Continue, "Continue"},
    {WebServer::HTTP_CODES::SwitchingProtocols, "Switching Protocols"},
    {WebServer::HTTP_CODES::Processing, "Processing"},
    {WebServer::HTTP_CODES::EarlyHints, "Early Hints"},
    {WebServer::HTTP_CODES::OK, "OK"},
    {WebServer::HTTP_CODES::Created, "Created"},
    {WebServer::HTTP_CODES::Accepted, "Accepted"},
    {WebServer::HTTP_CODES::NonAuthoritativeInformation, "Non Authoritative Information"},
    {WebServer::HTTP_CODES::NoContent, "No Content"},
    {WebServer::HTTP_CODES::ResetContent, "Reset Content"},
    {WebServer::HTTP_CODES::PartialContent, "Partial Content"},
    {WebServer::HTTP_CODES::MultiStatus, "Multi Status"},
    {WebServer::HTTP_CODES::AlreadyReported, "Already Reported"},
    {WebServer::HTTP_CODES::IMUsed, "IM Used"},
    {WebServer::HTTP_CODES::MultipleChoices, "Multiple Choices"},
    {WebServer::HTTP_CODES::MovedPermanently, "Moved Permanently"},
    {WebServer::HTTP_CODES::Found, "Found"},
    {WebServer::HTTP_CODES::SeeOther, "See Other"},
    {WebServer::HTTP_CODES::NotModified, "NotModified"},
    {WebServer::HTTP_CODES::TemporaryRedirect, "Temporary Redirect"},
    {WebServer::HTTP_CODES::PermanentRedirect, "Permanent Redirect"},
    {WebServer::HTTP_CODES::BadRequest, "Bad Request"},
    {WebServer::HTTP_CODES::Unauthorized, "Unauthorized"},
    {WebServer::HTTP_CODES::PaymentRequired, "Payment Required"},
    {WebServer::HTTP_CODES::Forbidden, "Forbidden"},
    {WebServer::HTTP_CODES::NotFound, "Not Found"},
    {WebServer::HTTP_CODES::MethodNotAllowed, "Method Not Allowed"},
    {WebServer::HTTP_CODES::NotAcceptable, "Not Acceptable"},
    {WebServer::HTTP_CODES::ProxyAuthenticationRequired, "Proxy Authentication Required"},
    {WebServer::HTTP_CODES::RequestTimeout, "Request Timeout"},
    {WebServer::HTTP_CODES::Conflict, "Conflict"},
    {WebServer::HTTP_CODES::Gone, "Gone"},
    {WebServer::HTTP_CODES::LengthRequired, "Length Required"},
    {WebServer::HTTP_CODES::PreconditionFailed, "Precondition Failed"},
    {WebServer::HTTP_CODES::ContentTooLarge, "Content Too Large"},
    {WebServer::HTTP_CODES::URITooLong, "URI Too Long"},
    {WebServer::HTTP_CODES::UnsupportedMediaType, "Unsupported Media Type"},
    {WebServer::HTTP_CODES::RangeNotSatisfiable, "Range Not Satisfiable"},
    {WebServer::HTTP_CODES::ExpectationFailed, "Expectation Failed"},
    {WebServer::HTTP_CODES::MisdirectedRequest, "Misdirected Request"},
    {WebServer::HTTP_CODES::UnprocessableContent, "Unprocessable Content"},
    {WebServer::HTTP_CODES::Locked, "Locked"},
    {WebServer::HTTP_CODES::FailedDependency, "Failed Dependency"},
    {WebServer::HTTP_CODES::TooEarly, "Too Early"},
    {WebServer::HTTP_CODES::UpgradeRequired, "Upgrade Required"},
    {WebServer::HTTP_CODES::PreconditionRequired, "Precondition Required"},
    {WebServer::HTTP_CODES::TooManyRequests, "Too Many Requests"},
    {WebServer::HTTP_CODES::RequestHeaderFieldsTooLarge, "Request Header Fields Too Large"},
    {WebServer::HTTP_CODES::UnavailableForLegalReasons, "Unavailable For Legal Reasons"},
    {WebServer::HTTP_CODES::InternalServerError, "Internal Server Error"},
    {WebServer::HTTP_CODES::NotImplemented, "Not Implemented"},
    {WebServer::HTTP_CODES::BadGateway, "Bad Gateway"},
    {WebServer::HTTP_CODES::ServiceUnavailable, "Service Unavailable"},
    {WebServer::HTTP_CODES::GatewayTimeout, "Gateway Timeout"},
    {WebServer::HTTP_CODES::HTTPVersionNotSupported, "HTTP Version Not Supported"},
    {WebServer::HTTP_CODES::VariantAlsoNegotiates, "Variant Also Negotiates"},
    {WebServer::HTTP_CODES::InsufficientStorage, "Insufficient Storage"},
    {WebServer::HTTP_CODES::LoopDetected, "Loop Detected"},
    {WebServer::HTTP_CODES::NetworkAuthenticationRequired, "Network Authentication Required"},
};

WebServer::HTTP_Request WebServer::Router::parse_raw_request(const std::string& raw_http_request){
    HTTP_Request request;
    std::istringstream ss(raw_http_request);

    std::string line;

    ss.getline(line.data(), 32, '\r');

    std::istringstream req_line_ss(line.data());

    // Firstly, parse the request line, which contains the method, route and HTTP version 
    std::string method;
    req_line_ss >> method >> request.path >> request.http_version;
    try {
        request.method = HTTP_METHOD_MAP.at(method);
    } catch (std::out_of_range err){
        request.method = METHOD_INVALID;
    }

    // Start parsing the headers
    while (std::getline(ss, line, '\r')){
        // Trim line from while-space and \r and \n characters
        line = trim(line);

        size_t colon_char_position = line.find_first_of(':', 0);
        if (colon_char_position == std::variant_npos){
            continue;
        }

        std::string header_key = trim(line.substr(0, colon_char_position));
        std::string header_value = trim(line.substr(colon_char_position + 1));

        // Check if the value for the header was already set, if so, simply append the value to the comma separated list
        try {
            std::string value = request.headers.at(header_key);
            if (value.at(value.size() - 1) == ','){
                value.append(header_value);
            } else {
                value.append(",");
                value.append(header_value);
            }

            request.headers[header_key] = value;
        } catch (std::out_of_range error){
            request.headers[header_key] = header_value;
        };
    }

    // Return early if request doesn't have body
    if (request.method == METHOD_GET || request.method == METHOD_HEAD){
        return request;
    }

    try {
        int content_length = std::atoi(request.headers.at("Content-Length").data());
        request.body = trim(raw_http_request.substr(raw_http_request.size() - content_length));
    } catch (std::out_of_range error){
        return request;
    };

    return request;
};


std::string WebServer::Router::generate_raw_http_response(std::string body, std::vector<std::string> headers, std::string http_version, WebServer::HTTP_CODES status_code, std::string status_text){
    std::string response = "";
    response.append(http_version + " ");
    response.append(std::to_string(int(status_code)) + " ");

    if (status_text.size() == 0){
        status_text = STATUS_TEXTS_MAP.at(status_code);
    } 

    response.append(status_text);
    response.append("\r\n");
    

    for (std::string& header: headers){
        response.append(header + "\r\n");
    };

    if (body.size() == 0){
        return response;
    }

    response.append("\r\n");

    response.append(body.begin(), body.end());

    return response;
};

void WebServer::Router::register_path(const std::string path, HTTP_METHODS method, Handler handler){
    m_paths[path][method] = Path{ .method = method, .path = path, .main_handler = handler, };
};

void WebServer::Router::GET(const std::string path, Handler handler){
    this->register_path(path, METHOD_GET, handler);
};

void WebServer::Router::HEAD(const std::string path, const Handler handler){
    this->register_path(path, METHOD_HEAD, handler);
};

void WebServer::Router::POST(const std::string path, const Handler handler){
    this->register_path(path, METHOD_POST, handler);
};

void WebServer::Router::PUT(const std::string path, const Handler handler){
    this->register_path(path, METHOD_PUT, handler);
};

void WebServer::Router::DELETE(const std::string path, const Handler handler){
    this->register_path(path, METHOD_DELETE, handler);
};

void WebServer::Router::CONNECT(const std::string path, const Handler handler){
    this->register_path(path, METHOD_CONNECT, handler);
};

void WebServer::Router::OPTIONS(const std::string path, const Handler handler){
    this->register_path(path, METHOD_OPTIONS, handler);
};

void WebServer::Router::TRACE(const std::string path, const Handler handler){
    this->register_path(path, METHOD_TRACE, handler);
};

void WebServer::Router::PATCH(const std::string path, const Handler handler){
    this->register_path(path, METHOD_PATCH, handler);
};
