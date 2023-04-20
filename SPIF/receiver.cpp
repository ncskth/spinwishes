#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>

using namespace std;

const uint16_t UDP_max_bytesize = 512;
const uint16_t ev_size = 4; // 4 bytes

const uint32_t P_SHIFT = 15;
const uint32_t Y_SHIFT = 0;
const uint32_t X_SHIFT = 16;
const uint32_t NO_TIMESTAMP = 0x80000000;

int main(int argc, char* argv[]) {

    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <port> <duration>" << endl;
        return 1;
    }

    int port = atoi(argv[1]);
    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }

    if (bind(udp_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error: Could not bind socket to port " << port << "." << endl;
        return 1;
    }

    cout << "Waiting for data..." << endl;

    uint32_t message[UDP_max_bytesize/ev_size];

    int total_ev_count = 0;
    
    int duration = atoi(argv[2]);
    auto start = std::chrono::steady_clock::now();
    while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < duration) {
        
        socklen_t clientAddrLen = sizeof(clientAddr);
        int bytesReceived = recvfrom(udp_fd, &message, sizeof(message), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived < 0) {
            cerr << "Error: Failed to receive data." << endl;
            break;
        }

        int numEvents = bytesReceived / ev_size;
        total_ev_count += numEvents;
        // for (int i = 0; i < numEvents; i++) {
        //     uint32_t ev = message[i];
        //     if ((ev & NO_TIMESTAMP) != NO_TIMESTAMP) {
        //         continue; // Ignore events with a timestamp
        //     }
        //     uint16_t x = (ev & 0x7FFF0000) >> X_SHIFT;
        //     uint16_t y = (ev & 0x00007FFF) >> Y_SHIFT;
        //     // cout << "Received event at (" << x << ", " << y << ")" << endl;
        // }
    }

    close(udp_fd);
    
    cout << "Total ev count: " << total_ev_count << endl;

    return 0;
}
