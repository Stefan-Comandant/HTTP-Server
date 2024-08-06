#include "../include/libs.h"
#include "../include/webserver.h"
#include <memory>
#include <vector>

void get_user(std::shared_ptr<WebServer::Context> ctx);
void post_user(std::shared_ptr<WebServer::Context> ctx);

struct User {
  std::string username;
  int age;
  std::string email;
  std::string password;
};

void to_json(json &j, const User &user) {
  j = json{
      {"email", user.email},
      {"password", user.password},
      {"username", user.username},
      {"age", std::to_string(user.age)},
  };
};

std::vector<User> users{User{
    .username = "Stefan",
    .age = 14,
    .email = "stefan@gmail.com",
    .password = "stefan",
}};

int main() {

  // Shit::Person p;
  // p.age = 14;
  // p.name = "Nigga";

  // json j = p;

  // std::cout << "Name: " << j.value("name", "").data() << '\n';

  // return 0;

  std::unique_ptr<WebServer::Router> router =
      std::make_unique<WebServer::Router>();

  router->set_file_source_directory("public");

  router->Use("/cors", {*WebServer::new_cors_middleware(WebServer::CORSConfig{
                           .allow_origins = "http://localhost:3000",
                           .allow_headers = "authorization",
                           .allow_credentials = true,

                       })});

  router->Use({(WebServer::logger_middleware)});

  router->Get("/", [](std::shared_ptr<WebServer::Context> ctx) {
    std::cout << "Lois, I\'m getting a request!\n";
    ctx->send_file("index.html");
  });

  router->Get("/hello", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->set_status(WebServer::OK)->send_file("hello.html");
  });

  router->Post("/say", [](std::shared_ptr<WebServer::Context> ctx) {
    // printf("Gettign a request at /say\n");

    std::string body = "You just said \"";
    body.append(ctx->request->body.data());
    body.append("\". What the fuck is wrong with you?");

    ctx->set_status(WebServer::OK)->send_string(body);
  });

  router->Get("/image", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->send_file("hello.png");
  });

  router->Get("/json", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->send_string("You have no dick, no balls and no bootyhole");
  });

  router->Get("/people/:name", [](std::shared_ptr<WebServer::Context> ctx) {
    std::cout << "The name " << ctx->param("name") << " is trash\n";

    ctx->send_file("spinner.html");
  });

  router->Get("/users/:userID/friends/:friendID",
              [](std::shared_ptr<WebServer::Context> ctx) {
                ctx->send_string("Friend with id " + ctx->param("friendID") +
                                 " of user with id " + ctx->param("userID"));
              });

  router->Get("/cookie", [](std::shared_ptr<WebServer::Context> ctx) {
    std::string cookie = ctx->cookies("old_cookie");
    std::cout << "Cookie value: " << cookie << '\n';

    ctx->send_string("Cookie");
  });

  router->Get("/cors", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->set_status(WebServer::HTTPCodes::OK)->send_string("CORS");
  });

  router->Get("/lorem", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->send_string("This is method GET");
  });

  router->Post("/lorem", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->send_string("This is method POST");
  });

  router->Get("/user", get_user);
  router->Post("/user", post_user);

  int error = router->listen(1234, "127.0.0.1");
  if (error == SOCKET_ERROR) {
    printf("ERROR::Listen failed: %s\n", WSAGetLastError());
  }

  return 0;
}

void get_user(std::shared_ptr<WebServer::Context> ctx) {
  json::array_t arr = json::array();
  // for (User user : users) {
  //   arr.push_back(user);
  // }

  arr.push_back(users);

  json json_data = arr;

  ctx->JSON(json_data);
};

void post_user(std::shared_ptr<WebServer::Context> ctx) {
  json js = json::parse(ctx->request->body);
  User user;

  user.email = js.value<std::string>("email", "name@example.com");
  user.password = js.value<std::string>("password", "123456789");
  user.username = js.value<std::string>("username", "");
  user.age = js.value<uint8_t>("age", -1);

  users.push_back(user);

  json j =
      json{{"status", "success"}, {"response", "Successfully added user!"}};

  ctx->JSON(json{
      {"status", "success"},
      {"response", "Successfully added user!"},
  });
};