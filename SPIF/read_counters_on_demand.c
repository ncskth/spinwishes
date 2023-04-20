#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "axi_address_and_register_map.h"

unsigned int *reg_bank;


void setup_reg_bank () {
  int fd = open ("/dev/mem", O_RDWR);

  if (fd < 1) {
    printf ("error: unable to open /dev/mem\n");
    exit (-1);
  }

  // map the register bank - 64KB address space
  reg_bank = (unsigned int *) mmap (
    NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, APB_BRIDGE
    );

  // close /dev/mem and drop root privileges
  close (fd);
  if (setuid (getuid ()) < 0) {
    exit (-1);
  }
}

struct PacketCounters
{
    int in_handled;
    int in_dropped;
    int in_total;
    int out_handled;
    int out_dropped;
    int out_total;
};


int main(int argc, char** argv) {

  printf("Ready to handle external requests\n");

  if (argc != 2) {
      std::cerr << "Usage: " << argv[0] << " <port>\n";
      return 1;
  }

  // create a UDP socket
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
      std::cerr << "Error creating socket\n";
      return 1;
  }

  // bind the socket to a port
  struct sockaddr_in servaddr;
  std::memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(std::stoi(argv[1]));
  if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
      std::cerr << "Error binding socket\n";
      return 1;
  }

  bool first_request = true;

  int previous_in_count = 0;
  int previous_dropped_in_count = 0;
  int previous_total_in_count = 0;

  int previous_out_count = 0;
  int previous_dropped_out_count = 0;
  int previous_total_out_count = 0;


  int current_in_count = 0;
  int current_dropped_in_count = 0;
  int current_total_in_count = 0;

  int current_out_count = 0;
  int current_dropped_out_count = 0;
  int current_total_out_count = 0;

  // receive and process incoming messages
  while (true) {
      char buffer[1024];
      struct sockaddr_in clientaddr;
      socklen_t clientaddrlen = sizeof(clientaddr);

      // receive a message from the socket
      int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientaddr, &clientaddrlen);
      if (n < 0) {
          std::cerr << "Error receiving message\n";
          return 1;
      } else {
        

        // setup memory-mapped register bank access and drop privileges
        setup_reg_bank ();

        current_in_count = reg_bank[67];
        current_dropped_in_count = reg_bank[66];
        current_total_in_count = current_in_count+current_dropped_in_count;
        current_out_count = reg_bank[65];
        current_dropped_out_count = reg_bank[64];
        current_total_out_count = current_out_count+current_dropped_out_count;

        if(current_in_count == 0){
          printf("SPIF has undergone reset\n");
          previous_in_count = 0;
          previous_dropped_in_count = 0;
          previous_total_in_count = 0;
          previous_out_count = 0;
          previous_dropped_out_count = 0;
          previous_total_out_count = 0;
        } 


        PacketCounters p_counters;
        p_counters.in_handled = current_in_count-previous_in_count;
        p_counters.in_dropped = current_dropped_in_count - previous_dropped_in_count;
        p_counters.in_total = current_total_in_count - previous_total_in_count;
        p_counters.out_handled = current_out_count - previous_out_count;
        p_counters.out_dropped = current_dropped_out_count - previous_dropped_out_count;
        p_counters.out_total = current_total_out_count - previous_total_out_count;


        int response[6] = {0,0,0,0,0,0};

        if(first_request){
          first_request = false;
          // printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\n","0",0,0,0,0,0,0);
        } else {

          // printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\n", 
          //         buffer, 
          //         p_counters.in_handled, 
          //         p_counters.in_dropped, 
          //         p_counters.in_total, 
          //         p_counters.out_handled, 
          //         p_counters.out_dropped, 
          //         p_counters.out_total);

          response[0] = p_counters.in_handled;
          response[1] = p_counters.in_dropped;
          response[2] = p_counters.in_total;
          response[3] = p_counters.out_handled;
          response[4] = p_counters.out_dropped;
          response[5] = p_counters.out_total;

        }

        sendto(sockfd, &response, sizeof(response), 0, (sockaddr*)&clientaddr, sizeof(clientaddr));
      

        previous_in_count = current_in_count;
        previous_dropped_in_count = current_dropped_in_count;
        previous_total_in_count = current_total_in_count;
        previous_out_count = current_out_count;
        previous_dropped_out_count = current_dropped_out_count;
        previous_total_out_count = current_total_out_count;
      }
  }

  return 0;
}
