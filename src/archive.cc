/* archive.cc: sample server program */
#include "server.h"
#include "connection.h"
#include "connectionclosedexception.h"

#include "service.h"

#include <memory>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]){
    if (argc < 2 || argc > 3) {
        cerr << "Usage: myserver port-number [/path/to/db/dir]" << endl;
        exit(1);
    }
    
    int port = -1;
    try {
        port = stoi(argv[1]);
    } catch (exception& e) {
        cerr << "Wrong port number. " << e.what() << endl;
        exit(1);
    }

    string path = "";
    if (argc == 3) {
        path = string(argv[2]);
    }
    
    Server server(port);
    if (!server.isReady()) {
        cerr << "Server initialization error." << endl;
        exit(1);
    }
    
    Service service(path);
    while (true) {
        auto conn = server.waitForActivity();
        if (conn != nullptr) {
            try {
                service.process(conn);
            } catch (ConnectionClosedException&) {
                server.deregisterConnection(conn);
                cout << "Client closed connection" << endl;
            }
        } else {
            conn = make_shared<Connection>();
            server.registerConnection(conn);
            cout << "New client connects" << endl;
        }
    }
}
