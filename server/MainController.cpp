#include "Server.cpp"
#include "RequestParser.cpp"
#include "Server_system-talk/process_starter.cpp"
#include <sys/signal.h>

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

public:

	MainController(int _port)
		: port(_port)
	{}
	
    void run() 
    {
        Server server(port);
        server.accept();
        while (true) 
        {			
            std::string request = server.read();
            
            if (request == "shutdown") {
                signal_handler(SIGTERM);
                server.write("Message: terminating signal was called");
            }
            
            if (!work)
				break;
            
            RequestBody body = parser.parse(request);
            
            // Running processes
            if (body.requestType == POST && body.args[0] == "process") 
            {
                int pid = run_process(body);
                if(pid == -1)
                    server.write(get_response(FAILED));
                else 
					server.write(get_response(OK));
            }
        }
    }
    
private:

	int port;
	bool work;
    Process process = Process();
    RequestParser parser = RequestParser();
    
    std::string get_response(ResponseType type) {
        switch (type) {
            case OK:
                return "HTTP/1.1 200 OK";
            case FAILED:
                return "HTTP/1.1 400 FAILED";
        }
        return "Oops! Something went wrong...\n";
    }

    int run_process(RequestBody requestBody) {
        std::vector<std::string> args = split(requestBody.body, ',');
        bool is_waiting;
        
        if (requestBody.args[1] == "true") 
			is_waiting = true;
        else if (requestBody.args[1] == "false") 
			is_waiting = false;
        else 
			return -1;
        
        int uid = stoi(args[2]);
        std::string command = args[3];
        
        int std_fd, err_fd;
        return process.run(is_waiting, uid, command, args, &std_fd, &err_fd);
    }
    
    void signal_handler(int sig)
    {
		if (sig == SIGTERM)
			work = false;
	}
};

int main(int argc, char *argv[])
{
	MainController controller(8080);
	controller.run();
	return 0;
}
