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
#include <fstream>


using namespace std;

const uint16_t UDP_max_bytesize = 1024;
const uint16_t ev_size = 4; // 4 bytes


const uint32_t P_SHIFT = 15;
const uint32_t Y_SHIFT = 0;
const uint32_t X_SHIFT = 16;
const uint32_t NO_TIMESTAMP = 0x80000000;



int send_request(char * ip_address, int port, int ev_per_sec) {

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
    destaddr.sin_addr.s_addr = inet_addr(ip_address); // IP address
    destaddr.sin_port = htons(port); // port number

    // send the message
    char message[10];
    std::sprintf(message, "%d", ev_per_sec);

    // char str[10];
    // std::sprintf(str, "%d", num);
    // return 0;

    int n = sendto(sockfd, message, std::strlen(message), 0, (struct sockaddr*)&destaddr, sizeof(destaddr));
    if (n < 0) {
        std::cerr << "Error sending message\n";
        return 1;
    }

    int res[6];
    memset(&res, 0, sizeof(res));
    // printf("Waiting for responses ... \n");
    recv(sockfd, &res, sizeof(res), 0);

    printf("%d,%d,%d,%d,%d,%d,%d\n", ev_per_sec, res[0], res[1], res[2], res[3], res[4], res[5]);


    std::ofstream file("my_data.csv", std::ios::app);
    file << ev_per_sec << "," << res[0] << "," << res[1]  << "," << res[3]  << "," << res[4] << std::endl;
    file.close();

    close(sockfd);

    return 0;
}

int main(int argc, char* argv[]) {


    srand(time(NULL)); // seed the random number generator with the current time

    if (argc != 8) {
        cerr << "Usage: " << argv[0] << " <x> <y> <t> <ip-address>:<port> <port_request> <duration> <ev_per_pack>" << endl;
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int sleeper = atoi(argv[3]);
    int port_request = atoi(argv[5]);
    int ev_per_pack = atoi(argv[7]);


    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(atoi(strrchr(argv[4], ':') + 1));
    char * spif_ip = strtok(argv[4], ":");
    inet_pton(AF_INET, spif_ip, &udp_addr.sin_addr);

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }

    int nb_pack_sent = 0;
    uint16_t message_sz = min(ev_per_pack, UDP_max_bytesize/ev_size);
    // printf("UDP_max_bytesize/ev_size: %d\n", UDP_max_bytesize/ev_size);
    // printf("ev_per_pack: %d\n", ev_per_pack);
    // printf("Message size: %d\n", message_sz);
    uint32_t message[message_sz]; // declare array

    int duration = atoi(argv[6]);
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
        nb_pack_sent += 1;
        // auto t_wait = sleeper-std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - current_t).count();
        // if (t_wait > 0){
        //     std::this_thread::sleep_for(std::chrono::microseconds(t_wait));
        // }


        // auto t_wait = sleeper-std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - current_t).count();
        // if (t_wait > 0){
        //     std::this_thread::sleep_for(std::chrono::nanoseconds(t_wait));
        // }
        
        if(sleeper>0){
            std::this_thread::sleep_for(std::chrono::microseconds(sleeper));
        }

    }

    int ev_per_sec = int(nb_pack_sent*message_sz/duration);
    // printf("Sleeper: %d\t Ev Count: %d\n", sleeper, ev_per_sec);
    // printf("sizeof(message): %ld\n", sizeof(message));

    
    send_request(spif_ip, port_request, ev_per_sec);

    close(udp_fd);
    return 0;
}
