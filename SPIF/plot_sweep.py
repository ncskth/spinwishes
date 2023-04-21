import csv
import matplotlib.pyplot as plt
import argparse

def parse_args():

    parser = argparse.ArgumentParser(description='Plotter')
    parser.add_argument('-fn', '--filename', type= str, help="File name", default="")

    return parser.parse_args()


if __name__ == '__main__':

    use_log_scale = True

    if use_log_scale:
        divider = 1
    else:
        divider = 1e6


    args = parse_args()


    # open the CSV file
    filename = args.filename

    with open(filename) as csvfile:
        csvreader = csv.reader(csvfile)


        # create empty lists to store the data
        ev_sent = []
        in_handled = []
        in_dropped = []
        in_total = []
        out_handled = []
        out_dropped = []
        out_total = []
        ratio_in = []
        ratio_out = []

        # iterate through each row in the CSV file
        for row in csvreader:
            print(row)
            # store the first column as ev_sent
            ev_sent.append(float(row[0])/divider)
            # store the remaining columns as y_data
            in_handled.append(float(row[1])/divider)
            in_dropped.append(float(row[2])/divider)
            out_handled.append(float(row[3])/divider)
            out_dropped.append(float(row[4])/divider)
            ratio_in.append(100*float(row[1])/float(row[0]))
            ratio_out.append(100*float(row[3])/float(row[0]))

    # y_data = [ev_sent, in_handled, in_dropped, out_handled, out_dropped]
    y_data = [ev_sent, in_handled, in_dropped]



    fig, ax = plt.subplots(figsize=(6,6))

    ax.set_aspect('equal')


    # plot the data
    ax.plot(ev_sent, ev_sent, label='Ev Sent', color= 'k', linestyle='--', linewidth=0.5)
    ax.scatter(ev_sent, in_handled, label='In Handled', alpha=1, color='g')
    ax.scatter(ev_sent, out_handled, label='Out Handled', alpha=0.3, color='b')
    ax.scatter(ev_sent, in_dropped, label='Dropped', alpha=0.5, color='r')

    # set the title and labels
    ax.set_title('SPIF: events handled and dropped vs total incoming events')
    ax.set_xlabel('Events streamed to SPIF [Mev/s]')
    ax.set_ylabel('Events handled/dropped by SPIF [Mev/s]')

    if use_log_scale:

        ax.set_xlim([1, 2e6])
        ax.set_ylim([1, 2e6])
        plt.xscale('log')
        plt.yscale('log')
    else:

        ax.set_xlim([0, 6])
        ax.set_ylim([0, 6])

    plt.grid(True)

    # show the legend
    plt.legend()

    # display the plot
    plt.savefig(f"{filename[:-4]}.png", dpi=300, bbox_inches='tight')

    plt.clf()

