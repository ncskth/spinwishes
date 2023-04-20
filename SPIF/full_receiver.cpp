#include <iostream>
#include <unistd.h>
#include <sys/un.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>


using namespace std;

const uint16_t UDP_max_bytesize = 512;
const uint16_t ev_size = 4; // 4 bytes


const uint32_t P_SHIFT = 15;
const uint32_t Y_SHIFT = 0;
const uint32_t X_SHIFT = 16;
const uint32_t NO_TIMESTAMP = 0x80000000;
using namespace std;


// int main(int argc, char* argv[]) {
//     if (argc != 2) {
//         cerr << "Usage: " << argv[0] << " <socket_path>" << endl;
//         return 1;
//     }

int main(int argc, char* argv[]) {


    srand(time(NULL)); // seed the random number generator with the current time

    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <port> <sock-path> <duration>" << endl;
        return 1;
    }


    int port = atoi(argv[1]);
    const char* socket_path = argv[2];
    int duration = atoi(argv[3]);


    struct sockaddr_un unx_addr;
    int unx_fd;

    /* Setting Up Unix Socket */
    unx_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unx_fd < 0) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }
    memset(&unx_addr, 0, sizeof(unx_addr));
    unx_addr.sun_family = AF_UNIX;
    strncpy(unx_addr.sun_path, socket_path, sizeof(unx_addr.sun_path)-1);
    if (connect(unx_fd, (struct sockaddr*)&unx_addr, sizeof(unx_addr)) < 0) {
        cerr << "Error: Could not connect to socket." << endl;
        return 1;
    }


    /* Setting Up UDP Socket*/
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


    int i = 0;
    while (true) {
        // Receive data from server
        char buffer[1024];
        int bytes_received = recv(unx_fd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            cerr << "Error: Failed to receive data." << endl;
            break;
        }
        else if (bytes_received == 0) {
            break;
        }
        else {
            buffer[bytes_received] = '\0';
            cout << "Ready to receive (c++)"  << endl;

        }

        
        int ev_sent = 0;
        uint16_t message_sz = UDP_max_bytesize/ev_size;
        uint32_t message[message_sz]; // declare array

        
        int total_ev_count = 0;
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





        /* Send data to python script */
        if (send(unx_fd, &total_ev_count, sizeof(total_ev_count), 0) < 0) {
            cerr << "Error: Failed to send data." << endl;
            break;
        }
        
        printf("-->%d \n", total_ev_count);

        sleep(1);
    }

    close(unx_fd);
    return 0;
}
