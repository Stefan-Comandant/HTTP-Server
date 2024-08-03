#include "../include/libs.h"
#include "../include/webserver.h"
// #include "../json/single_include/nlohmann/json.hpp"

// using nlohmann::json;

// struct Person {
//   std::string name;
//   int age;
//   void to_json(json &j) {
//     j = json{
//         {"name", this->name},
//         {"age", this->age},
//     };
//   }
// };

void middleware1(WebServer::Context *ctx) {
  std::cout << "This is the middleware1\n";
  std::cout << "This is middleware1: The name " << ctx->param("name")
            << " is a trash name\n";

  ctx->Next();
};

void middleware2(WebServer::Context *ctx) {
  std::cout << "This is the middleware2\n";
  std::cout << "This is middleware2: The name " << ctx->param("name")
            << " is a trash name\n";

  ctx->Next();
}

int main() {
  std::unique_ptr<WebServer::Router> router =
      std::make_unique<WebServer::Router>();

  router->set_file_source_directory("public");

  router->Use("/people", {middleware1, middleware2});

  router->Get("/", [](WebServer::Context *ctx) {
    std::cout << "Lois, I\'m getting a request!\n";
    ctx->send_file("index.html");
  });

  router->Get("/hello", [](WebServer::Context *ctx) {
    ctx->set_status(WebServer::OK)->send_file("hello.html");
  });

  router->Post("/say", [](WebServer::Context *ctx) {
    printf("Gettign a request at /say\n");

    std::string body = "You just said \"";
    body.append(ctx->request->body.data());
    body.append("\". What the fuck is wrong with you?");

    ctx->set_status(WebServer::OK)->send_string(body);
  });

  router->Get("/bitch",
              [](WebServer::Context *ctx) { ctx->send_file("bitches.png"); });

  router->Get("/bitchless/nigga", [](WebServer::Context *ctx) {
    ctx->send_string("You have no dick, no balls and no bootyhole");
  });

  router->Get("/people/:name", [](WebServer::Context *ctx) {
    std::cout << "The name " << ctx->param("name") << " is trash\n";

    ctx->send_file("./discrimination.html");
  });

  router->Get("/users/:userID/friends/:friendID", [](WebServer::Context *ctx) {
    ctx->send_string("Friend with id " + ctx->param("friendID") +
                     " of user with id " + ctx->param("userID"));
  });

  int error = router->listen(1234, "127.0.0.1");
  if (error == SOCKET_ERROR) {
    printf("ERROR::Listen failed: %s\n", WSAGetLastError());
  }

  // Person person;
  // person.age = 56;
  // person.name = "Adolf Hitler";

  // json j;
  // person.to_json(j);

  // printf("Value: %s\n", j.find("name").value().find("name")->value(""));
  // std::string name = j.find("name").value();
  // printf("Name: %s\n", name.data());

  // json::to_ubjson();
  return 0;
}
