#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <IP address> <port>\n";
        return 1;
    }

    // create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // set up the destination address
    struct sockaddr_in destaddr;
    std::memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = inet_addr(argv[1]); // IP address
    destaddr.sin_port = htons(std::stoi(argv[2])); // port number

    // send the message
    const char* message = "hola";
    int n = sendto(sockfd, message, std::strlen(message), 0, (struct sockaddr*)&destaddr, sizeof(destaddr));
    if (n < 0) {
        std::cerr << "Error sending message\n";
        return 1;
    }

    return 0;
}