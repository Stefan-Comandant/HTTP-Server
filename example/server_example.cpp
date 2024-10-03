#include "../include/fdwrapper.h"
#include "../include/router.h"

int main(){
    WebServer::Router router;
    router.listen(8080, "127.0.0.1");

    return 0;
}
