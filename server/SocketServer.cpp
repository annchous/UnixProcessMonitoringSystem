#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read

class SocketServer {
private:
    int port, addrlen;
    sockaddr_in sockaddr;
    int sockfd;
    int connection;
public:
    SocketServer(int port) : port(port) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            std::cout << "Failed to create socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        sockaddr_in sockaddr;
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = INADDR_ANY;
        sockaddr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
            std::cout << "Failed to bind to port " << port << ". errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        if (listen(sockfd, 10) < 0) {
            std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        addrlen = sizeof(sockaddr);
    }

    void accept() {
        connection = ::accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
        if (connection < 0) {
            std::cout << "Failed to grab connection. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::string read() const {
        char buffer[100];
        auto bytesRead = ::read(connection, buffer, 100);
        return std::string(buffer);
    }

    void write(const std::string& str) const {
        ::write(connection, str.c_str(), str.size());
    }
};