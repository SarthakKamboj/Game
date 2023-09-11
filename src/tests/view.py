import matplotlib.pyplot as plt
import csv
import sys
import numpy as np

if len(sys.argv) < 3:
    print("need to provide file path to csv only")
    exit()

filepath = sys.argv[1]
targets_filepath = sys.argv[2]
with open(filepath) as csvfile:
    with open(targets_filepath) as target_csvfile:
        reader = csv.reader(csvfile)
        target_reader = csv.reader(target_csvfile)
        xs = []
        ys = []
        target_xs = []
        target_ys = []
        next(reader)
        next(target_reader)
        for row in reader:
            time, x, y = row
            xs.append(float(x))
            ys.append(float(y))

        for row in target_reader:
            time, tx, ty = row
            target_xs.append(float(tx))
            target_ys.append(float(ty))
        # print(min(xs))
        # print(max(xs))
        num_ticks = 5
        xtick = np.linspace(min(min(xs), min(target_xs)), max(max(xs), max(target_xs)), num=num_ticks, dtype=np.dtype('float64'))
        ytick = np.linspace(min(min(ys), min(target_ys)), max(max(ys), max(target_ys)), num=num_ticks, dtype=np.dtype('float64'))
        plt.xticks(xtick)
        plt.yticks(ytick)
        plt.plot(xs, ys, label="smoothed")
        plt.plot(target_xs, target_ys, label="target")
        plt.legend()
        plt.savefig("transforms.png")