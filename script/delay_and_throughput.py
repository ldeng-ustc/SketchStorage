import os, sys
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

csv_dir = "./data/evaluations/"
img_dir = "./img/"

def autolabel(rects, xpos='center'):
    """
    Attach a text label above each bar in *rects*, displaying its height.

    *xpos* indicates which side to place the text w.r.t. the center of
    the bar. It can be one of the following {'center', 'right', 'left'}.
    """

    xpos = xpos.lower()  # normalize the case of the parameter
    ha = {'center': 'center', 'right': 'left', 'left': 'right'}
    offset = {'center': 0.5, 'right': 0.57, 'left': 0.43}  # x_txt = x + w*off

    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()*offset[xpos], 1.01*height,
                '{}'.format(height), ha=ha[xpos], va='bottom')


if __name__ == '__main__':
    sketchdb_throughput = []
    mysql_flow_throughput = []
    mysql_ts_throughput = []
    sketchdb_delay = []
    mysql_flow_delay = []
    mysql_ts_delay = []

    # SketchDB
    data = np.loadtxt(os.path.join(csv_dir, 'sketchdb', 'delay_PutFlowset.csv'), \
        dtype='double', delimiter=',')
    sketchdb_throughput.append(1 / data.mean() * 1e6)
    sketchdb_delay.append(np.percentile(data, 90))
    sketchdb_delay.append(np.percentile(data, 99))
    sketchdb_delay.append(data.max())

    data = np.loadtxt(os.path.join(csv_dir, 'sketchdb', 'delay_Scan.csv'), \
        dtype='double', delimiter=',')
    sketchdb_throughput.append(1 / data.mean() * 1e6)
    sketchdb_delay.append(np.percentile(data, 90))
    sketchdb_delay.append(np.percentile(data, 99))
    sketchdb_delay.append(data.max())

    data = np.loadtxt(os.path.join(csv_dir, 'sketchdb', 'delay_GetFlow.csv'), \
        dtype='double', delimiter=',')
    sketchdb_throughput.append(1 / data.mean() * 1e6)
    p90 = int(len(data) * 0.9) 
    sketchdb_delay.append(np.percentile(data, 90))
    sketchdb_delay.append(np.percentile(data, 99))
    sketchdb_delay.append(data.max())

    # MySQL Index by FlowKey
    data = np.loadtxt(os.path.join(csv_dir, 'mysql', 'delay_PutFlowset_flow.csv'), \
        dtype='double', delimiter=',')
    mysql_flow_throughput.append(1 / data.mean() * 1e6)
    p90 = int(len(data) * 0.9) 
    mysql_flow_delay.append(np.percentile(data, 90))
    mysql_flow_delay.append(np.percentile(data, 99))
    mysql_flow_delay.append(data.max())

    data = np.loadtxt(os.path.join(csv_dir, 'mysql', 'delay_Scan_flow.csv'), \
        dtype='double', delimiter=',')
    mysql_flow_throughput.append(1 / data.mean() * 1e6)
    p90 = int(len(data) * 0.9) 
    mysql_flow_delay.append(np.percentile(data, 90))
    mysql_flow_delay.append(np.percentile(data, 99))
    mysql_flow_delay.append(data.max())

    data = np.loadtxt(os.path.join(csv_dir, 'mysql', 'delay_GetFlow_flow.csv'), \
        dtype='double', delimiter=',')
    mysql_flow_throughput.append(1 / data.mean() * 1e6)
    p90 = int(len(data) * 0.9) 
    mysql_flow_delay.append(np.percentile(data, 90))
    mysql_flow_delay.append(np.percentile(data, 99))
    mysql_flow_delay.append(data.max())

    # MySQL Index by Timestamp
    data = np.loadtxt(os.path.join(csv_dir, 'mysql', 'delay_PutFlowset_sketch.csv'), \
        dtype='double', delimiter=',')
    mysql_ts_throughput.append(1 / data.mean() * 1e6)
    p90 = int(len(data) * 0.9) 
    mysql_ts_delay.append(np.percentile(data, 90))
    mysql_ts_delay.append(np.percentile(data, 99))
    mysql_ts_delay.append(data.max())

    data = np.loadtxt(os.path.join(csv_dir, 'mysql', 'delay_Scan_sketch.csv'), \
        dtype='double', delimiter=',')
    mysql_ts_throughput.append(1 / data.mean() * 1e6)
    p90 = int(len(data) * 0.9) 
    mysql_ts_delay.append(np.percentile(data, 90))
    mysql_ts_delay.append(np.percentile(data, 99))
    mysql_ts_delay.append(data.max())

    data = np.loadtxt(os.path.join(csv_dir, 'mysql', 'delay_GetFlow_sketch.csv'), \
        dtype='double', delimiter=',')
    mysql_ts_throughput.append(1 / data.mean() * 1e6)
    p90 = int(len(data) * 0.9) 
    mysql_ts_delay.append(np.percentile(data, 90))
    mysql_ts_delay.append(np.percentile(data, 99))
    mysql_ts_delay.append(data.max())

    sketchdb_delay = list(np.round(np.asarray(sketchdb_delay) / 1000, 3))
    mysql_flow_delay = list(np.round(np.asarray(mysql_flow_delay) / 1000, 3))
    mysql_ts_delay = list(np.round(np.asarray(mysql_ts_delay) / 1000, 3))

    print("Throughput:")
    print(f"sketchdb: {sketchdb_throughput}")
    print(f"mysql_flow: {mysql_flow_throughput}")
    print(f"mysql_ts: {mysql_ts_throughput}")
    print("Delay:")
    print(f"sketchdb: {sketchdb_delay}")
    print(f"mysql_flow: {mysql_flow_delay}")
    print(f"mysql_ts: {mysql_ts_delay}")
    

    ind = np.arange(len(sketchdb_throughput))  # the x locations for the groups
    width = 0.2  # the width of the bars

    fig, ax = plt.subplots()
    rects1 = ax.bar(ind - width * 1.5, sketchdb_throughput, width, label='SketchDB')
    rects2 = ax.bar(ind - width * 0.5, mysql_flow_throughput, width, label='MySQL id. flowkey')
    rects3 = ax.bar(ind + width * 0.5, mysql_ts_throughput, width, label='MySQL id. timestamp')
    
    #rects1 = ax.bar(ind - width/2, sketchdb_throughput, width,
    #                color='SkyBlue', label='SketchDB')
    #rects2 = ax.bar(ind + width/2, women_means, width,
    #                color='IndianRed', label='Women')

    # Add some text for labels, title and custom x-axis tick labels, etc.
    ax.set_ylabel('Throughput (Ops/s)')
    ax.set_xlabel('Operations')
    ax.set_xticks(ind)
    ax.set_xticklabels(('PutFlowset', 'Scan', 'GetFlow'))
    ax.legend()
    ax.set_ylim(1, 1000000)
    ax.set_yscale('log')

    # autolabel(rects1, "left")
    # autolabel(rects2, "right")

    plt.savefig(os.path.join(img_dir, 'throughput.png'))
    exit(0)
    data = np.loadtxt(os.path.join(csv_dir, 'sketchdb', 'throughput_PutFlowset.csv'))


    markers = ['o', 's', '^', 'p', 'D', 'v', 'h']
    data_arr_with_index = np.loadtxt(data_path, dtype='double', delimiter=',')
    data_arr_no_index = np.loadtxt(data_path_no_index, dtype='double', delimiter=',')
    
    data_arr = data_arr_with_index
    data_arr[:, 1] = (data_arr[:, 1] - data_arr[:, 1].min()) / 1000
    plt.plot(data_arr[:, 0], data_arr[:, 1], label='w hash table index', linestyle='--')
    data_arr = data_arr_no_index
    data_arr[:, 1] = (data_arr[:, 1] - data_arr[:, 1].min()) / 1000
    plt.plot(data_arr[:, 0], data_arr[:, 1], label='w/o hash table index', linestyle='--')
    data_arr = data_arr_with_index
    data_arr[:, 1] = data_arr_with_index[:, 1] - data_arr_no_index[:, 1] 
    plt.plot(data_arr[:, 0], data_arr[:, 1], label='hash table memory usage')
    plt.title('Memory Usage of SketchDB')
    plt.legend()
    plt.xlabel('Epoch (ms)')
    plt.ylabel('Memory Usage (MB)')
    plt.savefig(os.path.join(img_dir, 'sketchdb_index_memory_usage.png'))
    plt.cla()


    data_arr = data_arr_with_index
    data_arr = data_arr[data_arr[:, 2] < 1000]
    plt.plot(data_arr[:, 0], data_arr[:, 2], label='w hash table index', linestyle='--')
    data_arr = data_arr_no_index
    data_arr = data_arr[data_arr[:, 2] < 1000]
    plt.plot(data_arr[:, 0], data_arr[:, 2], label='w/o hash table index', linestyle='--')
    plt.title('Effect of Hash Table Index On Inserting Delay')
    plt.legend()
    plt.xlabel('Epoch (ms)')
    plt.ylabel('Delay (us)')
    plt.savefig(os.path.join(img_dir, 'sketchdb_index_cpu_usage.png'))
    data_arr = data_arr_no_index[:, 2]
    print(f"Insert w/o  Index: {(data_arr).mean()}")
    data_arr = data_arr_with_index[:, 2]
    print(f"Insert with Index: {(data_arr).mean()}")