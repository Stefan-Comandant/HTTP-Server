#include "../include/router.h"
#include "../include/fdwrapper.h"
#include "../include/context.h"

int main(){
    WebServer::Router router(WebServer::FD_Listen_Options{.non_blocking = true, });
    router.GET("/", [](WebServer::Context ctx){
            std::cout << "This is from the handler\n";
            });
    router.listen(1234, "127.0.0.1");
    return 0;
}
