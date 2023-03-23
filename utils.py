
import argparse

def parse_args():

    parser = argparse.ArgumentParser(description='SpiNNaker-SPIF Simulation')

    # SpiNNaker Simulation Parameters
    parser.add_argument('-nx', '--npc-x', type=int, help="# Neurons Per Core (x)", default=8)
    parser.add_argument('-ny', '--npc-y', type=int, help="# Neurons Per Core (y)", default=4)
    parser.add_argument('-r','--runtime', type=int, help="Run Time, in seconds", default=60*240)
    parser.add_argument('-d', '--dimension', type=int, help="1D or 2D", default=1)

    # SPIF parameters
    parser.add_argument('-i', '--spif-ip', type= str, help="SPIF's IP address", default="172.16.223.2")
    parser.add_argument('-pi', '--in-port', type=int, help="SPIF's port", default=3333)
    parser.add_argument('-po', '--out-port', type=int, help="SPIF's port", default=3332)    
    parser.add_argument('-x', '--width', type=int, help="Image width (in px)", default=640)
    parser.add_argument('-y', '--height', type=int, help="Image height (in px)", default=640)
    parser.add_argument('-sx', '--sub-width', type=int, help="SPIF's sub-width", default=16)
    parser.add_argument('-sy', '--sub-height', type=int, help="SPIF's sub-height", default=8)

    # 'Visualizer' Parameters (i.e a PC where to display SPIF's Output)
    parser.add_argument('-a', '--pc-ip', type= str, help="PC IP address", default="")
    parser.add_argument('-p', '--pc-port', type=int, help="PC port", default=0)    

    return parser.parse_args()