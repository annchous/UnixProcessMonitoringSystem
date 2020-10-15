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
			std::cout << "Enter command: ";
			scanf("%s", client_message);
			printf("%s", client_message);
		
			if (send(sock, client_message, 100, 0) < 0)
			{
				std::cout << "Sending failed\n";
				exit(0);
			}
		
			/*
			if (recv(sock, server_message, 100, 0) < 0)
			{
				std::cout << "Receiving failed\n";
				break;
			}
			
			std::cout << server_message << '\n';
			*/
			
			if (recv(sock, server_message, 100, 0) > 0)
				std::cout << server_message << '\n';
			
			if (!Connect()) exit(1);
		}
	
		close(sock);
	}
    
private:
	int sock;
	struct sockaddr_in server;
	
	char client_message[100];
	char server_message[100];
	
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
	
