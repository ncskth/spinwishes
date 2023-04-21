import argparse
import os
import socket
import struct
import threading
import queue
import random
import time
import numpy as np
import logging

logging.basicConfig(level=logging.INFO)


def parse_args():

    parser = argparse.ArgumentParser(description='SPIF testing')

    parser.add_argument('-ss', '--sender-sock-path', type= str, help="Sender Socket Path", default="/tmp/sender.sock")
    parser.add_argument('-rs', '--receiver-sock-path', type= str, help="Receiver Socket Path", default="/tmp/receiver.sock")

    return parser.parse_args()
   

def set_sender(sender_sock, inout_q, control_q):

    # t_array = np.concatenate((np.logspace(4,1, num=200, base=10), np.array([1])))
    t_array =  np.array([128, 64, 32, 16, 8, 4, 2, 1])

    # Delete the socket file if it already exists
    if os.path.exists(sender_sock):
        os.remove(sender_sock)

    # Create a Unix domain socket and bind to the specified path
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(sender_sock)
    sock.listen()

    # Wait for a client to connect
    logging.info(f"Waiting for a client to connect to {sender_sock}")
    conn, addr = sock.accept()
    logging.info(f"Connected to {sender_sock}")

    # Send and receive data
    for sleeper in t_array:

        ready_to_receive = 1
        control_q.put(ready_to_receive)
        time.sleep(1)
        logging.debug("Ready to send (Python) ")

        # Send data to client
        data_to_send = f"{int(sleeper)}"
        conn.send(data_to_send.encode())

        # Receive data from client
        data_received = conn.recv(1024)
        if not data_received:
            break
        ev_s = struct.unpack('i', data_received)[0]
        inout_q.put(['in', ev_s])
        time.sleep(1)
        logging.debug("Ready for next cycle (Python: sender side) ")
        time.sleep(1)

    # Clean up
    conn.close()
    sock.close()
    os.remove(sender_sock)


def set_receiver(receiver_sock, inout_q, control_q):

    # Delete the socket file if it already exists
    if os.path.exists(receiver_sock):
        os.remove(receiver_sock)

    # Create a Unix domain socket and bind to the specified path
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(receiver_sock)
    sock.listen()

    # Wait for a client to connect
    logging.info(f"Waiting for a client to connect to {receiver_sock}")
    conn, addr = sock.accept()
    logging.info(f"Connected to {receiver_sock}")

    # Send and receive data
    while True:
        ready_to_receive = 0
        while not control_q.empty():
            ready_to_receive = control_q.get(False)
        
        if ready_to_receive == 1:

            logging.debug("Ready to receive (Python) ")

            # Send data to client
            data_to_send = f"{int(ready_to_receive)}"
            conn.send(data_to_send.encode())

            # Receive data from client
            data_received = conn.recv(1024)
            if not data_received:
                break
            ev_s = struct.unpack('i', data_received)[0]
            inout_q.put(['out', ev_s])
        else:
            time.sleep(1)
        logging.debug("Ready for next cycle (Python: receiver side) ")

    # Clean up
    conn.close()
    sock.close()
    os.remove(sender_sock)
    
def get_summary(inout_q):

    while True:        
        while not inout_q.empty():
            data = inout_q.get(False)
            if data[0] == 'in':
                logging.debug(f"# events sent to SPIF: {data[1]}")
                logging.info(f"{data[1]} --> ")
            if data[0] == 'out':
                logging.debug(f"# events received from SPIF: {data[1]}")
                logging.info(f"               {data[1]}")
        time.sleep(1)

if __name__ == '__main__':

    args = parse_args()

    # Create a queue to exchange data between threads
    inout_q = queue.Queue()
    control_q = queue.Queue()

    # Create threads
    p_sender = threading.Thread(target=set_sender, args=(args.sender_sock_path, inout_q, control_q,))
    p_receiver = threading.Thread(target=set_receiver, args=(args.receiver_sock_path, inout_q, control_q,))
    p_summary = threading.Thread(target=get_summary, args=(inout_q,))

    # Start Threads
    p_sender.start()
    p_receiver.start()
    p_summary.start()

    # Wait for the threads to finish 
    p_sender.join()
    p_receiver.join()
    p_summary.join()

    
