#include "../include/router.h"

int main(){
    WebServer::Router router;
    router.listen(1234, "127.0.0.1");

    return 0;
}
