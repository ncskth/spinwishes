import spynnaker8 as p
from collections import defaultdict
import socket
from random import randint, random
from struct import pack
from time import sleep
from spinn_front_end_common.utilities.database import DatabaseConnection
from threading import Thread

# Device parameters are "pipe", "chip_coords", "ip_address", "port"
# Note: IP address and port are used to send in spikes when send_fake_spikes
#       is True
DEVICE_PARAMETERS = [(0, (0, 0), "172.16.223.2", 3333)]#,
#                     (0, (32, 16), "172.16.223.122", 3333),
#                     (0, (16, 8), "172.16.223.130", 3333)]
send_fake_spikes = True
sent_spikes = [defaultdict(list) for _ in range(len(DEVICE_PARAMETERS))]

# Used if send_fake_spikes is True
sleep_time = 0.1
n_packets = 100

# Run time if send_fake_spikes is False
run_time = 60000

if send_fake_spikes:
    run_time = (n_packets + 1) * sleep_time * 1000

# Constants
WIDTH = 100  # 640
HEIGHT = int(WIDTH * 3/4)  # 480
# Neurons per core
NEURONS_PER_CORE = 128
# Weight of connections between "layers"
WEIGHT = 5


def send_retina_input(ip_addr, port, i, spikes):
    """ This is used to send random input to the Ethernet listening in SPIF
    """
    NO_TIMESTAMP = 0x80000000
    min_x = 0
    min_y = 0
    max_x = WIDTH - 1
    max_y = HEIGHT - 1

    sleep(0.5 + (random() / 4.0))

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for _ in range(n_packets):
        n_spikes = randint(10, 100)
        data = b""
        for _ in range(n_spikes):
            x = randint(min_x, max_x)
            y = randint(min_y, max_y)
            neuron_id = (y * WIDTH) + x
            spikes[i][neuron_id].append(neuron_id)
            packed = NO_TIMESTAMP + neuron_id
            print(f"Sending x={x}, y={y}, packed={hex(packed)}")
            data += pack("<I", packed)
        sock.sendto(data, (ip_addr, port))
        sleep(sleep_time)


def start_fake_senders():
    global DEVICE_PARAMETERS

    for i, (_, _, ip_addr, port) in enumerate(DEVICE_PARAMETERS):
        t = Thread(target=send_retina_input,
                   args=(ip_addr, port, i, sent_spikes))
        t.start()


# Set up PyNN
p.setup(1.0, n_boards_required=24)

# Set the number of neurons per core to a rectangle
# (creates 512 neurons per core)
p.set_number_of_neurons_per_core(p.IF_curr_exp, NEURONS_PER_CORE)

if send_fake_spikes:
    # This is only used with the above to send data to the Ethernet
    connection = DatabaseConnection(start_fake_senders, local_port=None)

    # This is used with the connection so that it starts sending when the
    # simulation starts
    p.external_devices.add_database_socket_address(
        None, connection.local_port, None)


# These are our external retina devices connected to SPIF devices
devices = list()
captures = list()
for i, (pipe, chip_coords, _, _) in enumerate(DEVICE_PARAMETERS):
    dev = p.Population(None, p.external_devices.SPIFInputDevice(
        pipe=pipe, n_neurons=WIDTH * HEIGHT,
        n_neurons_per_partition=NEURONS_PER_CORE,
        chip_coords=chip_coords))

    # Create a population that captures the spikes from the input
    capture = p.Population(
        WIDTH * HEIGHT, p.IF_curr_exp(),
        label=f"Capture for device {i}")
    p.Projection(dev, capture, p.OneToOneConnector(),
                 p.StaticSynapse(weight=WEIGHT))

    # Record the spikes so we know what happened
    capture.record("spikes")

    # Save for later use
    devices.append(dev)
    captures.append(capture)

# Run the simulation for long enough for packets to be sent
p.run(run_time)

# Get out the spikes
capture_spikes = list()
for capture in captures:
    capture_spikes.append(capture.get_data("spikes").segments[0].spiketrains)

# Tell the software we are done with the board
p.end()

# Check if the data looks OK!
for i in range(len(devices)):

    # Go through the capture devices
    for neuron_id in range(WIDTH * HEIGHT):

        if send_fake_spikes and (len(sent_spikes[i][neuron_id]) !=
                                 len(capture_spikes[i][neuron_id])):
            print(f"{neuron_id}: {sent_spikes[i][neuron_id]}"
                  f" != {capture_spikes[i][neuron_id]}")

print("Passed!")