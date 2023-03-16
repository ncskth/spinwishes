import numpy as np
import math
import pyNN.spiNNaker as p
import pdb
import socket
from struct import pack
import matplotlib.pyplot as plt
import argparse
import time


def parse_args():

    parser = argparse.ArgumentParser(description='SpiNNaker-SPIF Simulation')

    # SpiNNaker Simulation Parameters
    parser.add_argument('-s', '--size', type=int, help="# pixels on each dimension", default=1) 
    parser.add_argument('-ox', '--x-offset', type=int, help="x offset", default=0) 
    parser.add_argument('-oy', '--y-offset', type=int, help="y offset", default=0) 
    parser.add_argument('-d', '--pc-ip', type= str, help="PC IP address", default="10.37.222.3")
    parser.add_argument('-p', '--pc-port', type=int, help="PC port", default=3331)    

    return parser.parse_args()
   

if __name__ == '__main__':


    args = parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    P_SHIFT = 15
    Y_SHIFT = 0
    X_SHIFT = 16
    NO_TIMESTAMP = 0x80000000
    # print(np_spikes.shape)
    while True:
        data = b""
        for i in range(args.size):
            for j in range(args.size):                
                x = i + args.x_offset
                y = j + args.y_offset
                polarity = 1
                packed = (NO_TIMESTAMP + (polarity << P_SHIFT) + (y << Y_SHIFT) + (x << X_SHIFT))
                data += pack("<I", packed)
                sock.sendto(data, (args.pc_ip, args.pc_port))
        time.sleep(5)