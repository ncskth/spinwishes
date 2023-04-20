import numpy as np
import argparse
import csv

def parse_args():

    parser = argparse.ArgumentParser(description='SpiNNaker-SPIF Simulation')

    parser.add_argument('-x', '--width', type=int, help="Image width (in px)", default=128)
    parser.add_argument('-y', '--height', type=int, help="Image height (in px)", default=128)
    parser.add_argument('-r', '--repetitions', type=int, help="# of repetitions", default=1)
    parser.add_argument('-f', '--filename', type= str, help="File name", default="ev_list.csv")


    return parser.parse_args()
   

if __name__ == '__main__':


    args = parse_args()

    # generate the random arrays

    nb_pixels = args.width*args.height

    single_ev_list = np.zeros((nb_pixels,2), dtype=int)
    composed_ev_list = np.zeros((nb_pixels*args.repetitions,2), dtype=int)


    ev_idx = 0
    for x in range(args.width):
        for y in range(args.height):
            single_ev_list[ev_idx, 0] = x 
            single_ev_list[ev_idx, 1] = y 
            ev_idx += 1

    for i in range(args.repetitions):
        composed_ev_list[i*nb_pixels:(i+1)*nb_pixels,:] = np.random.permutation(single_ev_list)

    # write the arrays to a csv file
    with open(args.filename, 'w', newline='') as file:
        writer = csv.writer(file)
        for ev_idx in range(composed_ev_list.shape[0]):
            writer.writerow([composed_ev_list[ev_idx, 0], composed_ev_list[ev_idx, 1]])