#include "SocketServer.cpp"
#include "RequestParser.cpp"
#include "Server_system-talk/process_starter.hpp"

using namespace std;

enum ResponseType {
    OK = 200,
    FAILED = 400
};

// Requests:
// POST /process/${wait_enabled}/${UID}/${command} HTTP/1.1
// Body: text/application;
// GET
// /ps

class MainController {
private:
    Process process = Process();
    RequestParser parser = RequestParser();
    string get_response(ResponseType type) {
        switch (type) {
            case OK:
                return "HTTP/1.1 200 OK";
            case FAILED:
                return "HTTP/1.1 400 FAILED";
        }
    }

    int run_process(RequestBody requestBody) {
        vector<string> args = split(requestBody.body, ',');
        bool is_wait;
        if(requestBody.args[1]=="true") is_wait = true;
        else if(requestBody.args[1]=="false") is_wait = false;
        else return -1;
        int uid = stoi(args[2]);
        string command = args[3];
        int std_fd, err_fd;
        return process.run(is_wait, uid, command, args, &std_fd, &err_fd);
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
            RequestBody body = parser.parse(request);
            // Running processes
            if(body.requestType==POST && body.args[0]=="process") {
                int pid = run_process(body);
                if(pid==-1)
                    server.write(get_response(FAILED));
                else {
                    server.write(get_response(OK));
                }
            }
        }
    }
};