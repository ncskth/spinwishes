#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>


using namespace std;

const uint16_t UDP_max_bytesize = 1024;
const uint16_t ev_size = 4; // 4 bytes


const uint32_t P_SHIFT = 15;
const uint32_t Y_SHIFT = 0;
const uint32_t X_SHIFT = 16;
const uint32_t NO_TIMESTAMP = 0x80000000;

int main(int argc, char* argv[]) {


    srand(time(NULL)); // seed the random number generator with the current time

    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " <x> <y> <t> <ip-address>:<port> <duration> <ev_per_pack>" << endl;
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int sleeper = atoi(argv[3]);
    int ev_per_pack = atoi(argv[6]);


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

    int ev_sent = 0;
    uint16_t message_sz = min(ev_per_pack, UDP_max_bytesize/ev_size);
    printf("UDP_max_bytesize/ev_size: %d\n", UDP_max_bytesize/ev_size);
    printf("ev_per_pack: %d\n", ev_per_pack);
    printf("Message size: %d\n", message_sz);
    uint32_t message[message_sz]; // declare array

    int duration = atoi(argv[5]);
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
    printf("Sleeper: %d\t Ev Count: %d\n", sleeper,int(ev_sent*message_sz/duration));
    // printf("sizeof(message): %ld\n", sizeof(message));

    close(udp_fd);
    return 0;
}
