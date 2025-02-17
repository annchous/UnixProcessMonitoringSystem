#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <signal.h>

class Server 
{
private:
    int port;
    int addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in serveraddr; 
    struct sockaddr_in clientaddr;
    int sockfd;
    int connection;

public:

    Server(int port)
		: port(port) 
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) 
		{
			std::cout << "Error: Failed to create socket. errno: " << errno << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "Socket was created\n";

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = INADDR_ANY;
		serveraddr.sin_port = htons(port);

		if (bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) 
		{
			std::cout << "Error: Failed to bind to port " << port << ". errno: " << errno << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "Binding was finished successfully\n";

		if (listen(sockfd, 10) < 0) 
		{
			std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
			exit(EXIT_FAILURE);
		}
    }

    void accept() {
        connection = ::accept(sockfd, (struct sockaddr*)&clientaddr, (socklen_t*)&addrlen);
        if (connection < 0) 
        {
            std::cout << "Failed to grab connection. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    
    int getConnectionSocket() const
    {
		return connection;
	}
	
	void setPort(int port)
	{
		this->port = port;
	}
};
