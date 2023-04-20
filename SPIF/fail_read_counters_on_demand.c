
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>


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


void save_to_csv(double x, double y, double z)
{
    std::ofstream file("data.csv", std::ios::app);
    file << x << "," << y << "," << z << std::endl;
    file.close();
}

int main(int argc, char** argv) {

  printf("Waiting for external request :) \n");

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

        // printf("Results for Current Request");
        // printf("\nInput:\n");
        // printf("Periph. Packets:  %u\n", current_in_count-previous_in_count);
        // printf("Dropped Packets:  %u\n", current_dropped_in_count - previous_dropped_in_count);
        // printf("Total Packets:  %u\n", current_total_in_count - previous_total_in_count);            
        
        // printf("\nOutput:\n");
        // printf("Periph. Packets:  %u\n", current_out_count - previous_out_count);
        // printf("Dropped Packets:  %u\n", current_dropped_out_count - previous_dropped_out_count);
        // printf("Total Packets:  %u\n", current_total_out_count - previous_total_out_count);

        // printf("\n\n");


        if(first_request){
          first_request = false;
          printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\n","0",0,0,0,0,0,0);
        } else {
          printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\n", 
                  buffer, 
                  current_in_count-previous_in_count, 
                  current_dropped_in_count - previous_dropped_in_count, 
                  current_total_in_count - previous_total_in_count, 
                  current_out_count - previous_out_count, 
                  current_dropped_out_count - previous_dropped_out_count, 
                  current_total_out_count - previous_total_out_count);
        }
      

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

        // printf("Results for Current Request");
        // printf("\nInput:\n");
        // printf("Periph. Packets:  %u\n", current_in_count-previous_in_count);
        // printf("Dropped Packets:  %u\n", current_dropped_in_count - previous_dropped_in_count);
        // printf("Total Packets:  %u\n", current_total_in_count - previous_total_in_count);            
        
        // printf("\nOutput:\n");
        // printf("Periph. Packets:  %u\n", current_out_count - previous_out_count);
        // printf("Dropped Packets:  %u\n", current_dropped_out_count - previous_dropped_out_count);
        // printf("Total Packets:  %u\n", current_total_out_count - previous_total_out_count);

        // printf("\n\n");


        if(first_request){
          first_request = false;
          printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\n","0",0,0,0,0,0,0);
        } else {
          printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\n", 
                  buffer, 
                  current_in_count-previous_in_count, 
                  current_dropped_in_count - previous_dropped_in_count, 
                  current_total_in_count - previous_total_in_count, 
                  current_out_count - previous_out_count, 
                  current_dropped_out_count - previous_dropped_out_count, 
                  current_total_out_count - previous_total_out_count);
        }
      

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