#include "../include/fdwrapper.h"
#include "../include/router.h"
#include <cstdio>
#include <iostream>
#include <vector>

int main(){
    FD_Wrapper fd;

    fd.socket();
    
    fd.listen(1234, "127.0.0.1");

    FD_Wrapper client = fd.accept();

    std::vector<char> buf(1024);
    int rv = client.read(buf.data(), 1024);
    if (rv == -1){
        perror("Failed to read data from client");
        return 1;
    }

    std::cout << "Data: \n**************************\n" << buf.data() << "\n**************************\n";

    std::string wbuf = "Hello world!";
    client.write(wbuf.data(), wbuf.size());

    return 0;
}
