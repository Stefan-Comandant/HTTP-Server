#include "../include/libs.h"
#include "../include/webserver.h"
#include <memory>

void cors_middleware(std::shared_ptr<WebServer::Context> ctx) {
  ctx->add_header("Access-Control-Allow-Origin", "http://localhost:3000");
  ctx->add_header("Access-Control-Allow-Credentials", "true");
  ctx->add_header("Access-Control-Allow-Headers", "authorization");

  if (ctx->request->method.compare("OPTIONS") != 0) {
    ctx->Next();
    return;
  }

  ctx->set_status(WebServer::NoContent)->send_string("");
}

int main() {
  std::unique_ptr<WebServer::Router> router =
      std::make_unique<WebServer::Router>();

  router->set_file_source_directory("public");

  router->Use("/cors", {cors_middleware});

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
    std::cout << "Doing the CORS shit\n";
  });

  router->Get("/lorem", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->send_string("This is method GET");
  });

  router->Post("/lorem", [](std::shared_ptr<WebServer::Context> ctx) {
    ctx->send_string("This is method POST");
  });

  int error = router->listen(1234, "127.0.0.1");
  if (error == SOCKET_ERROR) {
    printf("ERROR::Listen failed: %s\n", WSAGetLastError());
  }

  return 0;
}
