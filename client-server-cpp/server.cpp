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

class Server
{
	public:
	
	Server(int port)
	{
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(port);
	}
	
	void CreateSocket()
	{
		socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
		if (socket_descriptor == -1)
		{
			std::cout << "Error: Cannot create socket\n";
		}
		std::cout << "Socket was created\n";
	}
	
	void Bind()
	{
		if (bind(socket_descriptor, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			std::cout << "Error: Binding failed\n";
			exit(0);
		}
		std::cout << "Binding was finished successfully\n";
	}
	
	void Accept()
	{
		client_socket = accept(socket_descriptor, (struct sockaddr *)&client, (socklen_t*)&size_of_addrstruct);
		if (client_socket < 0)
		{
			std::cout << "Error: Accept failed\n";
			exit(0);
		}
		std::cout << "Connection was accepted successfully\n";
	}
	
	void Run()
	{
		CreateSocket();
		Bind();
		
		listen(socket_descriptor, 3);
		
		Accept();
		
		int data_size;
		char client_message[100];
		char server_reply[100];
		
		while ((data_size = recv(client_socket, client_message, 100, 0)) > 0)
		{
			server_reply[0] = 0;
			strncpy(server_reply, client_message, strlen(client_message));
			
			write(client_socket, server_reply, 100);
		}
		
		if (data_size == 0)
		{
			std::cout << "Client disconnected\n";
			exit(0);
		}
		else if (data_size == -1)
		{
			std::cout << "Receiving failed\n";
		}
	}
	
	private:
	
	int socket_descriptor;
	int client_socket;
	
	struct sockaddr_in server;
	struct sockaddr_in client;
	
	const int size_of_addrstruct = sizeof(struct sockaddr_in);
};

int main()
{
	Server server(2020);
	server.Run();
	
	return 0;
}
