#include "../include/libs.h"
#include "../include/webserver.h"

void middleware(WebServer::Context *ctx) {
  std::cout << "This is middleware: The name " << ctx->param("name")
            << " is a trash name\n";

  ctx->Next();
};

int main() {
  std::unique_ptr<WebServer::Router> router =
      std::make_unique<WebServer::Router>();

  router->set_file_source_directory("public");

  router->Use("/people", {middleware});

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

  router->Get("/image",
              [](WebServer::Context *ctx) { ctx->send_file("bitches.png"); });

  router->Get("/json", [](WebServer::Context *ctx) {
    ctx->send_string("You have no dick, no balls and no bootyhole");
  });

  router->Get("/people/:name", [](WebServer::Context *ctx) {
    std::cout << "The name " << ctx->param("name") << " is trash\n";

    ctx->send_file("./spinner.html");
  });

  router->Get("/users/:userID/friends/:friendID", [](WebServer::Context *ctx) {
    ctx->send_string("Friend with id " + ctx->param("friendID") +
                     " of user with id " + ctx->param("userID"));
  });

  router->Get("/cookie", [](WebServer::Context *ctx) {
    std::string cookie = ctx->cookies("old_cookie");
    std::cout << "Cookie value: " << cookie << '\n';

    ctx->send_string("Cookie");
  });

  int error = router->listen(1234, "127.0.0.1");
  if (error == SOCKET_ERROR) {
    printf("ERROR::Listen failed: %s\n", WSAGetLastError());
  }

  return 0;
}
