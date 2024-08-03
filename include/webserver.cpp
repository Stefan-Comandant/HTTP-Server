#include "webserver.h"
#include <vector>

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

std::string WebServer::make_response_body(
    const WebServer::HTTPCodes status_code, const std::string body,
    const std::map<std::string, std::vector<std::string>> headers,
    const std::string content_type) {
  std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " +
                         status_texts.at(status_code) + "\r\n" +
                         "Content-Type: " + content_type +
                         "\r\nContent-Length: " + std::to_string(body.size());

  for (std::pair<std::string, std::vector<std::string>> pair : headers) {
    for (std::string header_value : pair.second) {
      response.append("\r\n" + pair.first + ": ");
      response.append(header_value);
    }
  }

  response.append("\r\n\r\n" + body);

  return response;
};

std::string WebServer::make_response_body(
    const WebServer::HTTPCodes status_code, const std::vector<char> body,
    const std::map<std::string, std::vector<std::string>> headers,
    const std::string content_type) {
  std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " +
                         status_texts.at(status_code) + "\r\n" +
                         "Content-Type: " + content_type +
                         "\r\nContent-Length: " + std::to_string(body.size());

  for (std::pair<std::string, std::vector<std::string>> pair : headers) {
    for (std::string header_value : pair.second) {
      response.append("\r\n" + pair.first + ": ");
      response.append(header_value);
    }
  }

  response.append("\r\n\r\n");
  response.append(body.begin(), body.end());

  return response;
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
