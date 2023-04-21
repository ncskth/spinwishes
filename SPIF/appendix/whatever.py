import socket

UDP_IP = "172.16.223.98"  # replace with the IP address you want to listen to
UDP_PORT = 3332  # replace with the port number you want to listen to

# create a socket object
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

try:
    # bind the socket to a specific IP address and port
    sock.bind((UDP_IP, UDP_PORT))
    print("Listening for incoming UDP messages on {}:{}".format(UDP_IP, UDP_PORT))

    # listen for incoming UDP messages
    while True:
        data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
        print("Received message:", data.decode())

except OSError as e:
    print("Error binding to {}:{}".format(UDP_IP, UDP_PORT))
    print(e)



