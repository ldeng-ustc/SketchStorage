import os, sys
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

csv_dir = "./data/evaluations/sketchdb/"
img_dir = "./img/"

data_path = os.path.join(csv_dir, 'profile_std.csv')
data_path_no_index = os.path.join(csv_dir, 'profile_fake.csv')
data_path_ns = os.path.join(csv_dir, 'profile_ns.csv')

if __name__ == '__main__':
    markers = ['o', 's', '^', 'p', 'D', 'v', 'h']
    data_arr_with_index = np.loadtxt(data_path, dtype='double', delimiter=',')
    data_arr_no_index = np.loadtxt(data_path_no_index, dtype='double', delimiter=',')
    data_arr_ns = np.loadtxt(data_path_ns, dtype='double', delimiter=',')
    
    data_arr = data_arr_with_index
    data_arr[:, 1] = (data_arr[:, 1] - data_arr[:, 1].min()) / 1000
    plt.plot(data_arr[:, 0], data_arr[:, 1], label='w index and switching', linestyle='-')
    data_arr = data_arr_no_index
    data_arr[:, 1] = (data_arr[:, 1] - data_arr[:, 1].min()) / 1000
    plt.plot(data_arr[:, 0], data_arr[:, 1], label='w/o index', linestyle='--')
    data_arr = data_arr_ns
    data_arr[:, 1] = (data_arr[:, 1] - data_arr[:, 1].min()) / 1000
    plt.plot(data_arr[:, 0], data_arr[:, 1], label='w index, w/o switching', linestyle='-.')


    # plt.title('Memory Usage of SketchDB')
    plt.legend()
    plt.xlabel('Epoch (ms)')
    plt.ylabel('Memory Usage (MB)')
    plt.savefig(os.path.join(img_dir, 'sketchdb_index_memory_usage.png'))
    plt.cla()


    data_arr = data_arr_with_index
    data_arr = data_arr[data_arr[:, 2] < 1000]
    plt.plot(data_arr[:, 0], data_arr[:, 2], label='w index and switching', linestyle='-')
    data_arr = data_arr_no_index
    data_arr = data_arr[data_arr[:, 2] < 1000]
    plt.plot(data_arr[:, 0], data_arr[:, 2], label='w/o index', linestyle='--')
    data_arr = data_arr_ns
    data_arr = data_arr[data_arr[:, 2] < 1000]
    plt.plot(data_arr[:, 0], data_arr[:, 2], label='w index, w/o switching', linestyle='-.')
    # plt.title('Effect of Hash Table Index On Inserting Delay')
    plt.legend()
    plt.xlabel('Epoch (ms)')
    plt.ylabel('Delay (us)')
    plt.savefig(os.path.join(img_dir, 'sketchdb_index_cpu_usage.png'))
    data_arr = data_arr_no_index[:, 2]
    print(f"Insert w/o  Index: {(data_arr).mean()}")
    data_arr = data_arr_with_index[:, 2]
    print(f"Insert with Index: {(data_arr).mean()}")
    data_arr = data_arr_ns[:, 2]
    print(f"Insert w/o Switching: {(data_arr).mean()}")