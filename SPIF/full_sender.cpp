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

    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " <x> <y> <t> <ip-unx_address>:<port> <sock-path> <duration>" << endl;
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int sleeper = atoi(argv[3]);

    const char* socket_path = argv[5];
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
    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(atoi(strrchr(argv[4], ':') + 1));
    inet_pton(AF_INET, strtok(argv[4], ":"), &udp_addr.sin_addr);

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }



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
            cout << "Ready to send (c++)"  << endl;
        }

        
        int ev_sent = 0;
        uint16_t message_sz = UDP_max_bytesize/ev_size;
        uint32_t message[message_sz]; // declare array

        int duration = atoi(argv[6]);
        sleeper = atoi(buffer);
        auto start_t = std::chrono::steady_clock::now();
        while(true) {
            auto current_t = std::chrono::steady_clock::now();
            if(std::chrono::duration_cast<std::chrono::seconds>(current_t - start_t).count() > duration){
                break;
            }
            

            // assign values to elements using a for loop
            for (int i = 0; i < message_sz; i++) {
                
                uint16_t x = (uint16_t) (rand() % width);
                uint16_t y = (uint16_t) (rand() % height);
                uint32_t n = (NO_TIMESTAMP + (1 << P_SHIFT) + (y << Y_SHIFT) + (x << X_SHIFT));
                message[i] = n; // assign value to element i 
            }

            // std::cout << "hello" << std::endl;
            if (sendto(udp_fd, &message, sizeof(message), 0, (struct sockaddr*)&udp_addr, sizeof(udp_addr)) < 0) {
                cerr << "Error: Failed to send data." << endl;
                return 1;
            }
            ev_sent += 1;

            auto t_wait = sleeper-std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - current_t).count();
            if (t_wait > 0){
                std::this_thread::sleep_for(std::chrono::microseconds(t_wait));
            }
        }
        int total_ev_count = int(ev_sent*UDP_max_bytesize/ev_size);
        printf("Sleeper: %d\t Ev Count: %d\n", sleeper, total_ev_count);
        // Send data to server
        if (send(unx_fd, &total_ev_count, sizeof(total_ev_count), 0) < 0) {
            cerr << "Error: Failed to send data." << endl;
            break;
        }

        sleep(1);
    }

    close(unx_fd);
    return 0;
}
