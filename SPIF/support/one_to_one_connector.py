import numpy as np
import math
import pyNN.spiNNaker as p
import pdb
import socket
from struct import pack
import matplotlib.pyplot as plt
import time
import utils


def create_conn_list(width, height, weight):
    '''
    The idea here is to map neurons from presynaptic population to postsynaptic population
    Both populations must have the same number of neurons (width*height) 
    '''
    conn_list = []
    delay = 1 # ms

    for idx in range(width*height):
        pre_idx = idx
        post_idx = idx
        conn_list.append((pre_idx, post_idx, weight, delay))
        
    return conn_list

def create_lut(w, h, sh, sw):
        
    delay = 1 # 1 [ms]
    nb_col = math.ceil(w/sw)
    nb_row = math.ceil(h/sh)

    lut = np.zeros((w*h,2), dtype='uint16')

    lut_ix = 0
    for h_block in range(nb_row):
        for v_block in range(nb_col):
            for row in range(sh):
                for col in range(sw):
                    x = v_block*sw+col
                    y = h_block*sh+row
                    if x<w and y<h:
                        lut[lut_ix] = [x, y]
                        lut_ix += 1

    return lut

class Computer:

    def __init__(self, args):

        # SpiNNaker simulation parameters
        self.npc_x = args.npc_x
        self.npc_y = args.npc_y
        self.runtime = 1000*args.runtime
        self.dimension = int(args.dimension)

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

        # Connectivity
        self.weight = 8

        # Visualizer
        self.pc_ip = args.pc_ip
        self.pc_port = args.pc_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.lut = create_lut(self.width, self.height, self.npc_x, self.npc_y)

        self.print_data()


    def __enter__(self):

        p.setup(timestep=1.0, n_boards_required=24)
        p.set_number_of_neurons_per_core(p.IF_curr_exp, (self.npc_x, self.npc_y))



        ###############################################################################################################
        # Set SPIF Input
        ###############################################################################################################
        IN_POP_LABEL = "input"
        print("Using SPIFRetinaDevice")
        input_pop = p.Population(self.width * self.height, p.external_devices.SPIFRetinaDevice(
                                pipe=self.pipe, width=self.width, height=self.height, 
                                sub_width=self.sub_width, sub_height=self.sub_height, 
                                chip_coords=self.chip), label=IN_POP_LABEL)

        ###############################################################################################################
        # Set Middle Layer
        ###############################################################################################################
        
        MID_POP_LABEL = "middle"

        if self.dimension == 1:
            mid_pop = p.Population(self.width*self.height, p.IF_curr_exp(), label=MID_POP_LABEL)
        else:
            mid_pop = p.Population(self.width*self.height, p.IF_curr_exp(), structure=p.Grid2D(self.width / self.height), label=MID_POP_LABEL)
        p.Projection(input_pop, mid_pop, p.OneToOneConnector(), receptor_type='excitatory')


        ###############################################################################################################
        # Set SPIF Output
        ###############################################################################################################

        OUT_POP_LABEL = "output"

        print("Using SPIFOutputDevice")
        conn = p.external_devices.SPIFLiveSpikesConnection([MID_POP_LABEL],self.spif_ip, self.spif_out_port)
        conn.add_receive_callback(MID_POP_LABEL, self.recv_spif)
        output_pop = p.Population(None, p.external_devices.SPIFOutputDevice(
            database_notify_port_num=conn.local_port, chip_coords=self.chip), label=OUT_POP_LABEL)
        p.external_devices.activate_live_output_to(mid_pop, output_pop)


    def print_data(self):

        message = "Simulation Summary:\n"
        message += f"   - runtime: {self.runtime} seconds\n"
        message += f"   - dimension: {self.dimension}D\n"
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

        data = b""
        if self.pc_ip == "":
            pass
        else: 
            P_SHIFT = 15
            Y_SHIFT = 0
            X_SHIFT = 16
            NO_TIMESTAMP = 0x80000000
            np_spikes = np.array(spikes)
            # print(np_spikes.shape)
            for i in range(np_spikes.shape[0]):
                x = self.lut[np_spikes[i]][0]
                y = self.lut[np_spikes[i]][1]
                polarity = 1
                print(f"{np_spikes[i]} --> ({self.lut[np_spikes[i]][0]}, {self.lut[np_spikes[i]][1]})")
                packed = (NO_TIMESTAMP + (polarity << P_SHIFT) + (y << Y_SHIFT) + (x << X_SHIFT))
                data += pack("<I", packed)
            self.sock.sendto(data, (self.pc_ip, self.pc_port))


    def run_sim(self):
        p.run(self.runtime)


    def __exit__(self, e, b, t):
        p.end()


   

if __name__ == '__main__':


    args = utils.parse_args()
    spin = Computer(args)


    with spin:
        spin.run_sim()