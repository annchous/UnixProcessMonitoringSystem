#include "Server.cpp"
#include "RequestParser.cpp"
#include "Server_system-talk/process_starter.cpp"
#include "Server_system-talk/logger_wrapper.cpp"
#include "Server_system-talk/process_monitor_wrapper.cpp"
#include <csignal>

enum ResponseType {
    OK = 200,
    FAILED = 400
};

// Requests:
// POST /process/${wait_enabled}/${UID}/${command} HTTP/1.1
// Body: text/application;
// GET
// ps

class MainController {

public:

    MainController(int _port)
            : port(_port)
            , work(true)
    {}

    void run() 
    {
		Server server = Server(port);
        server.accept();
        
        std::vector<std::string> logfiles = {"logs/main.log"};
        Log log(logfiles);
        Monitor monitor;
        
        int pid = fork();
        if (pid != 0)
            exit(0);
        close(0);
        close(1);
        close(2);
        setsid();
        
        while (true) 
        {	
			write(server.getConnectionSocket(), "Enter command: ", 100);
			char req[100];
			while(true)
			{
				//write(server.getConnectionSocket(), "CYCLE", 100);
				if (read(server.getConnectionSocket(), req, 100) > 0)
					break;
			}
            std::string request = req;
            
            if (request == "shutdown") {
                signal_handler(SIGTERM);
                write(server.getConnectionSocket(), "shutdown", 100);
            }
            
            if (!work)
				break;
            
            //write(server.getConnectionSocket(), "Checkpoint: after work checking", 100);
            RequestBody body = parser.parse(request);
            //write(server.getConnectionSocket(), "Checkpoint: after parsing", 100);
            
            // Running processes
            if (body.requestType == POST && body.args[0] == "process") 
            {
				//std::cout << "Checkpoint: before run_process";
                int pid = run_process(body, server);
                //std::cout << "Checkpoint: after run_process";
                
                if (pid == -1)
                    write(server.getConnectionSocket(), get_response(FAILED).c_str(), 100);
                else 
					write(server.getConnectionSocket(), get_response(OK).c_str(), 100);
            }
            //write(server.getConnectionSocket(), "BIGCYCLE", 100);

            if(body.requestType == GET && body.args[0] == "ps") 
            {
				//write(server.getConnectionSocket(), "Checkpoint: before get_processes call", 100);
                get_processes(monitor, server);
            }
        }
    }

private:

    int port;
	bool work;
	char client_request[100];
    RequestParser parser = RequestParser();
    Runnable process = Runnable();

    void get_processes(Monitor &monitor, Server &server) {
		write(server.getConnectionSocket(), "Checkpoint: get_processes", 100);
        std::vector<Process> response = monitor.getProcesses();
        write(server.getConnectionSocket(), get_response(OK).c_str(), 100);
        for(auto proc : response) {
            std::string proc_stat = std::to_string(proc.UID) + " "
                    + std::to_string(proc.PID) + " " + std::string(proc.cpu) + " "
                    + std::string(proc.memory) + " " + std::to_string(proc.VSZ) + " "
                    + std::to_string(proc.RSS) + " " + std::to_string(proc.TT) + " "
                    + std::string(proc.status) + " " + std::string(proc.start_time) + " "
                    + std::string(proc.command);
            write(server.getConnectionSocket(), proc_stat.c_str(), proc_stat.length());
        }
    }

    std::string get_response(ResponseType type) 
    {
        switch (type) {
            case OK:
                return "HTTP/1.1 200 OK";
            case FAILED:
                return "HTTP/1.1 400 FAILED";
        }
        return "Oops! Something went wrong...\n";
    }

    int run_process(RequestBody requestBody, Server &server) 
    {
		write(server.getConnectionSocket(), "Enter arguments: ", 100);
		char rbody[100];
		while(true)
		{
			//write(server.getConnectionSocket(), "Checkpoint: reading arguments in run_process", 100);
			if (read(server.getConnectionSocket(), rbody, 100) > 0)
				break;
		}
		requestBody.body = rbody;
		
		
        std::vector<std::string> args = RequestParser::split(requestBody.body, ',');
        
        bool is_waiting;
        //write(server.getConnectionSocket(), "Checkpoint: run_process 1", 100);
        if (requestBody.args[1] == "true") 
			is_waiting = true;
        else if (requestBody.args[1] == "false") 
			is_waiting = false;
        else 
			return -1;
			
        //write(server.getConnectionSocket(), "Checkpoint: run_process 2", 100);
        int uid = atoi(requestBody.args[2].c_str());
        //write(server.getConnectionSocket(), "Checkpoint: run_process 3", 100);
        std::string command = requestBody.args[3];
        
        int std_fd, err_fd;
        //write(server.getConnectionSocket(), "Checkpoint: run_process 4", 100);
        
        int code = process.run(is_waiting, uid, command, args, &std_fd, &err_fd);
        
        char buffer_out[100] = {};
		char buffer_err[100] = {};
		std::string out, erro, buff;
		while (read(std_fd, buffer_out, 100) != 0)
		{
			buff = buffer_out;
			out += buff;
			//write(server.getConnectionSocket(), "Checkpoint: CYCLE", 100);
			read(std_fd, buffer_out, 100);
		}
		while(read(err_fd, buffer_err, 100) != 0)
		{
			buff = buffer_err;
			erro += buff;
			//write(server.getConnectionSocket(), "Checkpoint: CYCLE", 100);
			read(err_fd, buffer_out, 100);
		}
		
		//write(server.getConnectionSocket(), out.c_str(), out.length());
		//write(server.getConnectionSocket(), erro.c_str(), erro.length());
		//write(server.getConnectionSocket(), "Checkpoint: before exit from run_process", 100);
		return code;
    }
    
    void signal_handler(int sig)
    {
		if (sig == SIGTERM)
			work = false;
	}
	
};
