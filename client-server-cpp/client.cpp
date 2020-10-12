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


class Client
{
public:

	Client(int port)
	{
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_port = htons(port);
	}
	
	void CreateSocket()
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1)
		{
			std::cerr << "Error: Cannot create socket\n";
		}
		std::cout << "Socket was created\n";
	}
	
	void Connect()
	{
		if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		std::cerr << "Error: Connection failed\n";
		exit(0);
	}
	std::cout << "Connected\n";
	}
	
	void Run()
	{
		CreateSocket();
		Connect();
		
		while (true)
		{
			std::cout << "Enter command: ";
			std::cin >> client_message;
		
			if (send(sock, client_message, 100, 0) < 0)
			{
				std::cout << "Sending failed\n";
				exit(0);
			}
		
			if (recv(sock, server_message, 100, 0) < 0)
			{
				std::cout << "Receiving failed\n";
				break;
			}
			
			std::cout << server_message << '\n';
		}
	
		close(sock);
	}
	
private:
	int sock;
	struct sockaddr_in server;
	
	char client_message[100];
	char server_message[100];
	
};

int main()
{
	Client client(2020);
	client.Run();
	
	return 0;
}
	
