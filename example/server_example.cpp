#include "../include/router.h"
#include "../include/fdwrapper.h"

int main(){
    WebServer::Router router(FD_Listen_Options{.non_blocking = true, });
    router.listen(1234, "127.0.0.1");
    return 0;
}
