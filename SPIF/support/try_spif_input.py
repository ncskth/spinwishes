import socket
import struct
import time
import multiprocessing

import numpy as np

import pyNN.spiNNaker as pynn
from pyNN.space import Grid2D

from spinn_front_end_common.utilities.database.database_connection import DatabaseConnection
from spynnaker.pyNN.spynnaker_external_device_plugin_manager import SpynnakerExternalDevicePluginManager


# False to use Spike Source Array
USE_SPIF = True


neuron_type_class = pynn.IF_curr_exp
neuron_type = neuron_type_class(
    tau_m=5.0,
    cm=5.0,
    v_rest=0.0,
    v_reset=0.0,
    v_thresh=0.5,
    tau_refrac=1.0,
    i_offset=0.0,
    v=0.0,
)


def main():
    pynn.setup(timestep=1)

    input_shape = (8, 4)
    input_height, input_width = input_shape
    num_neurons = input_height * input_width

    sub_width = 4
    sub_height = 4

    pynn.set_number_of_neurons_per_core(neuron_type_class, (sub_width, sub_height))

    ss_input_spikes = np.array((
        (), (), (), (),
        (), (), (), (),
        (), (), (), (),
        (20.0,), (), (), (),
        (20.0,), (), (), (),
        (), (), (), (),
        (), (), (), (),
        (), (), (), (),
    ), dtype=object)

    spif_input_spikes = (
        (20, np.array((
            (0, 0, 0, 0),
            (0, 0, 0, 0),
            (0, 0, 0, 0),
            (1, 0, 0, 0),
            (1, 0, 0, 0),
            (0, 0, 0, 0),
            (0, 0, 0, 0),
            (0, 0, 0, 0),
            ))),
    )

    if USE_SPIF:
        spif_pipe = 0

        spif_retina_device = pynn.external_devices.SPIFRetinaDevice(
            pipe=spif_pipe,
            width=input_width,
            height=input_height,
            sub_width=sub_width,
            sub_height=sub_height,
            input_x_shift=16,
            input_y_shift=0,
            chip_coords=(0, 0),
            base_key=None,
            board_address=None)

        spif_input_neurons = pynn.Population(
            num_neurons,
            spif_retina_device,
            label='SPIF input neurons')

        has_started = multiprocessing.Semaphore(0)

        process = multiprocessing.Process(
            target=send_spikes_spif,
            args=(has_started, spif_input_spikes))

        process.start()

        def start_callback():
            has_started.release()

        print("\nUsing SPIF")
        run(spif_input_neurons, input_shape, start_callback)

    else:
        spike_source = pynn.SpikeSourceArray(spike_times=ss_input_spikes)

        ss_input_neurons = pynn.Population(
            num_neurons,
            spike_source,
            label='Spike source input neurons',
            structure=Grid2D(input_width / input_height))

        print("\nUsing Spike Source Array")
        run(ss_input_neurons, input_shape)


def run(input_neurons, input_shape, start_callback=None):
    kernel = np.array(((2.0,),))

    connector = pynn.ConvolutionConnector(kernel)

    output_height, output_width = connector.get_post_shape(input_shape)
    num_output_neurons = output_width * output_height

    conv_neurons = pynn.Population(
        num_output_neurons,
        neuron_type,
        label='Convolutional neurons',
        structure=Grid2D(output_width / output_height))

    pynn.Projection(input_neurons, conv_neurons, connector, pynn.Convolution())

    conv_neurons.record(('spikes',))

    if start_callback is not None:
        conn = DatabaseConnection(
            start_resume_callback_function=start_callback, local_port=None)
        SpynnakerExternalDevicePluginManager.add_database_socket_address(
            conn.local_ip_address, conn.local_port, None)

    duration = 50
    pynn.run(duration)

    output_data = conv_neurons.get_data()

    pynn.end()

    segments = output_data.segments[0]
    spiketrains = segments.spiketrains

    for y in range(output_height):
        print(f"y={y}:\t", end='')
        for x in range(output_width):
            neuron_index = y * output_width + x
            spiketrain = spiketrains[neuron_index]
            print(f"x={x}:{{", end='')
            for t in spiketrain:
                print(f"{int(t)} ", end='')
            print("}\t", end='')
        print()


def send_spikes_spif(has_started, spikes):
    has_started.acquire()

    start_time = time.perf_counter_ns()

    for i, (frame_time, frame) in enumerate(spikes):
        time_now = time.perf_counter_ns()
        time_offset = (time_now - start_time) / 1000_000
        time_until_next = frame_time - time_offset

        if time_until_next > 0:
            sleep_time = time_until_next / 1000
            time.sleep(sleep_time)

        send_to_spif(frame)


def send_to_spif(frame):
    spif_ip_address = "172.16.223.98"
    spif_port = 3333

    active_pixel_coords = (frame > 0).nonzero() * np.array(((1,), (1,)))

    NO_TIMESTAMP = 0x80000000
    Y_SHIFT = 0
    X_SHIFT = 16

    packed_pixels = (
        NO_TIMESTAMP + (y << Y_SHIFT) + (x << X_SHIFT)
        for y, x in active_pixel_coords.T
    )

    num_pixels = active_pixel_coords.shape[1]
    data = struct.pack("<" + "I" * num_pixels, *packed_pixels)

    if len(data):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(data, (spif_ip_address, spif_port))


if __name__ == "__main__":
    main()
