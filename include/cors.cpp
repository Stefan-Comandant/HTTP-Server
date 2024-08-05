#include "libs.h"
#include "webserver.h"

WebServer::PathHandler *
WebServer::new_cors_middleware(WebServer::CORSConfig config) {
  PathHandler *handler = new PathHandler(
      [config](std::shared_ptr<WebServer::Context> ctx) -> void {
        bool should_apply_middleware = true;

        if (config.filter != nullptr)
          should_apply_middleware = (*config.filter)(ctx);
        else
          should_apply_middleware =
              (ctx->request->method.compare("OPTIONS") == 0);

        ctx->add_header("Access-Control-Allow-Origin", config.allow_origins);
        ctx->add_header("Access-Control-Allow-Methods", config.allow_methods);
        ctx->add_header("Access-Control-Allow-Headers", config.allow_headers);
        ctx->add_header("Access-Control-Max-Age:",
                        std::to_string(config.max_age));
        ctx->add_header("Access-Control-Expose-Headers:",
                        config.expose_headers);

        if (config.allow_credentials) {
          ctx->add_header("Access-Control-Allow-Credentials", "true");
        }

        if (!should_apply_middleware) {
          ctx->Next();
          return;
        }

        ctx->set_status(HTTPCodes::NoContent)->send_string("");
      });

  return handler;
};
