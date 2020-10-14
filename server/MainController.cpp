#include "SocketServer.cpp"
#include "RequestParser.cpp"

using namespace std;

enum ResponseType {
    OK = 200,
    FAILED = 400;
};

class MainController {
private:
    RequestParser parser = RequestParser();
    string get_response(ResponseType type) {
        switch (type) {
            case OK:
                return "HTTP/1.1 200 OK";
            case FAILED:
                return "HTTP/1.1 400 FAILED";
        }
    }
public:
    void run(int port) {
        SocketServer server(port);
        server.accept();
        while(true) {
            string request = server.read();
            if(request == "shutdown") {
                break;
            }
            bool is_successful = false;
            // Do something idk
            server.write(get_response(is_successful ? OK : FAILED));
        }
    }
};