#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Error: Please provide a socket path as an argument." << endl;
        return 1;
    }

    const char* socket_path = argv[1];
    struct sockaddr_un addr;
    int unx_fd;

    unx_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unx_fd < 0) {
        cerr << "Error: Could not create socket." << endl;
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(unx_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Error: Could not connect to socket." << endl;
        return 1;
    }

    int i = 0;
    while (true) {
        i++;
        if (send(unx_fd, &i, sizeof(i), 0) < 0) {
            cerr << "Error: Failed to send data." << endl;
            break;
        }
        cout << "Sent " << i << endl;
        sleep(1);
    }

    close(unx_fd);
    return 0;
}