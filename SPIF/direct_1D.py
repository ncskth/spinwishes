import numpy as np
import math
import pyNN.spiNNaker as p
import pdb
import socket
from struct import pack
import matplotlib.pyplot as plt
import argparse
import time
import os


class Computer:

    def __init__(self, args):

        # SpiNNaker simulation parameters
        self.npc_x = args.npc_x
        self.npc_y = args.npc_y
        self.runtime = 1000*args.runtime

        # SPIF parameters
        self.width = args.width
        self.height = args.height
        self.sub_width = args.sub_width
        self.sub_height = args.sub_height
        self.chip = (0,0)
        self.spif_in_port = args.in_port
        self.pipe = self.spif_in_port - 3333
        self.spif_ip = args.spif_ip
        self.spif_out_port = args.out_port
        print(f"SPIF @ {args.spif_ip}")

        # Visualizer
        self.pc_ip = args.pc_ip
        self.pc_port = args.pc_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        self.print_data()


    def __enter__(self):

        p.setup(timestep=1.0, n_boards_required=24)
        p.set_number_of_neurons_per_core(p.IF_curr_exp, (self.npc_x, self.npc_y))



        ###############################################################################################################
        # Set SPIF Input
        ###############################################################################################################
        IN_POP_LABEL = "input"
        print("Using SPIFRetinaDevice")
        # input_pop = p.Population(self.width * self.height, p.external_devices.SPIFRetinaDevice(
        #                         pipe=self.pipe, width=self.width, height=self.height, 
        #                         sub_width=self.sub_width, sub_height=self.sub_height, 
        #                         chip_coords=self.chip), label=IN_POP_LABEL)


        input_pop = p.Population(None, p.external_devices.SPIFInputDevice(
                                pipe=self.pipe, n_neurons=self.width*self.height,
                                n_neurons_per_partition=self.npc_x*self.npc_y,
                                chip_coords=self.chip), label=IN_POP_LABEL)

        MID_POP_LABEL = "middle"
        middle_pop = p.Population(self.width*self.height, p.IF_curr_exp(), label=MID_POP_LABEL)
        p.Projection(input_pop, middle_pop, p.OneToOneConnector(), p.StaticSynapse(weight=5))


        ###############################################################################################################
        # Set SPIF Output
        ###############################################################################################################

        OUT_POP_LABEL = "output"

        print("Using SPIFOutputDevice")
        conn = p.external_devices.SPIFLiveSpikesConnection([MID_POP_LABEL],self.spif_ip, self.spif_out_port)
        conn.add_receive_callback(MID_POP_LABEL, self.recv_spif)
        output_pop = p.Population(None, p.external_devices.SPIFOutputDevice(
            database_notify_port_num=conn.local_port, chip_coords=self.chip), label=OUT_POP_LABEL)
        p.external_devices.activate_live_output_to(middle_pop, output_pop)


    def print_data(self):

        message = "Simulation Summary:\n"
        message += f"   - runtime: {self.runtime} seconds\n"
        message += f"   - with {self.npc_x}*{self.npc_y} neurons per core\n"
        message += f"   - SPIF @{self.spif_ip}\n"
        message += f"      - input port: {self.spif_in_port}\n"
        message += f"      - output port: {self.spif_out_port}\n"
        message += f"      - width: {self.width}\n"
        message += f"      - height: {self.height}\n"
        message += f"      - subwidth: {self.sub_width}\n"
        message += f"      - subheight: {self.sub_height}\n"
        if self.pc_ip != "":
            message += f"   - Client @{self.pc_ip}, port {self.pc_port}\n"
        message += "\n Is this correct? "
        print(message)
        time.sleep(5)


    def recv_spif(self, label, spikes):

        np_spikes = np.array(spikes)
        if self.pc_ip == "":
            for i in range(np_spikes.shape[0]):
                print(f"Receving event from neuron id: {np_spikes[i]}")
            # pass
        else:        
            data = b""

            P_SHIFT = 15
            Y_SHIFT = 0
            X_SHIFT = 16
            NO_TIMESTAMP = 0x80000000
            # print(np_spikes.shape)
            for i in range(np_spikes.shape[0]):
                polarity = 1
                x = int(np_spikes[i] % self.width)
                y = int(np_spikes[i] / self.width)
                print(f"{np_spikes[i]} --> ({x},{y})")
                packed = (NO_TIMESTAMP + (polarity << P_SHIFT) + (y << Y_SHIFT) + (x << X_SHIFT))
                data += pack("<I", packed)
            self.sock.sendto(data, (self.pc_ip, self.pc_port))


    def run_sim(self):
        p.run(self.runtime)


    def __exit__(self, e, b, t):
        p.end()

spin_spif_map = {"1": "172.16.223.2", 
                 "37": "172.16.223.106", 
                 "43": "172.16.223.98",
                 "13": "172.16.223.10",
                 "121": "172.16.223.122",
                 "129": "172.16.223.130"}

def parse_args():

    parser = argparse.ArgumentParser(description='SpiNNaker-SPIF Simulation')

    # SpiNNaker Simulation Parameters
    parser.add_argument('-nx', '--npc-x', type=int, help="# Neurons Per Core (x)", default=8)
    parser.add_argument('-ny', '--npc-y', type=int, help="# Neurons Per Core (y)", default=4)
    parser.add_argument('-r','--runtime', type=int, help="Run Time, in seconds", default=60*240)
    parser.add_argument('-b', '--board', type= str, help="SpiNN-5 Board IP x.x.x.<X>", default="1")

    # SPIF parameters
    parser.add_argument('-pi', '--in-port', type=int, help="SPIF's port", default=3333)
    parser.add_argument('-po', '--out-port', type=int, help="SPIF's port", default=3332)    
    parser.add_argument('-x', '--width', type=int, help="Image width (in px)", default=128)
    parser.add_argument('-y', '--height', type=int, help="Image height (in px)", default=128)
    parser.add_argument('-sx', '--sub-width', type=int, help="SPIF's sub-width", default=16)
    parser.add_argument('-sy', '--sub-height', type=int, help="SPIF's sub-height", default=8)

    # 'Visualizer' Parameters (i.e a PC where to display SPIF's Output)
    parser.add_argument('-d', '--pc-ip', type= str, help="PC IP address", default="")
    parser.add_argument('-p', '--pc-port', type=int, help="PC port", default=0)    

    return parser.parse_args()
   

if __name__ == '__main__':

    args = parse_args()
    args.spif_ip = spin_spif_map[args.board]


    try:
        # pdb.set_trace()
        os.system("clear")
        rig_command = f"rig-power 172.16.223.{int(args.board)-1}"
        print(f"Currently waiting for '{rig_command}' to end")
        os.system(rig_command)
    except:
        print("Wrong SpiNN-5 to SPIF mapping")
        quit()

    spin = Computer(args)


    with spin:
        spin.run_sim()