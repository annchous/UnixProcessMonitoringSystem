#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <iostream>
#include <exception>


class Client
{
public:

	Client(int port)
	{
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_port = htons(port);
		
		CreateSocket();
		if (!Connect()) exit(1);
	}
	
	void Run()
	{
		while (true)
		{
			
			if (recv(sock, server_message, 100, 0) > 0)
			{
				if (strncmp(server_message, "Enter command: ", 100) == 0)
				{
					std::cout << server_message;
					gets(command);
					send(sock, command, 100, 0);
				}
				else if (strncmp(server_message, "Enter arguments: ", 100) == 0)
				{
					std::cout << server_message;
					gets(arguments);
					send(sock, arguments, 100, 0);
				}
				else if (strncmp(server_message, "shutdown", 100) == 0)
				{
					std::cout << "Message: terminating signal was called\n";
					exit(0);
				}
				else
					std::cout << server_message << std::endl;
			}
			
		}
	
		close(sock);
	}
    
private:
	int sock;
	struct sockaddr_in server;
	
	char client_message[100];
	char server_message[100];
	char arguments[100];
	char command[100];
	
	void CreateSocket()
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1)
		{
			std::cerr << "Error: Cannot create socket\n";
		}
		std::cout << "Socket was created\n";
	}
	
	bool Connect()
	{
		if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		std::cerr << "Error: Connection failed\n";
		return false;
	}
		std::cout << "Connected\n";
		return true;
	}
	
};

int main(int argc, char *argv[])
{
	Client client(8080);
	client.Run();
	
	return 0;
}
	
