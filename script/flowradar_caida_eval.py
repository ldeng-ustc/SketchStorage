import os, sys
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

exec_dir = "./build/apps/eval/"
exec_file = "flowradar_caida_eval"
exec_path = os.path.join(exec_dir, exec_file)

csv_dir = "./data/evaluations/flowradar_caida_eval/"
img_dir = "./img/"

K = 5
N = 1000
L = 100
R = 2000
S = 100


def get_name(k=K, l=L, r=R, s=S, n=N):
    return f"{exec_file}_k={k}_m={m}_n={n}"

def load_or_run(k, m, n):
    csv_file = get_name(k, m, n) + '.csv'
    csv_path = os.path.join(csv_dir, csv_file)
    if not os.path.exists(csv_path):
        cmd = f"{exec_path} -o {csv_path} -k {k} -m {m} -n {n}"
        os.system(cmd)
    return np.loadtxt(csv_path, dtype='int32', delimiter=',')
    

if __name__ == '__main__':
    os.makedirs(exec_dir, exist_ok=True)
    os.makedirs(csv_dir, exist_ok=True)
    os.makedirs(img_dir, exist_ok=True)

    markers = ['o', 's', '^', 'p', 'D', 'v', 'h']
    for k in range(1, K+1):
        success_rate = []
        for m in range(L, R+1, S):
            arr = load_or_run(k, m, N)
            success_rate.append((arr[: , 0] == 0).sum() / N)
        plt.plot(range(L, R+1, S), success_rate, label=f'k={k}', marker=markers[k - 1])

    plt.title('FlowRadar Decoding Success Rate on CAIDA')
    plt.legend()
    plt.xlabel('Number of Table Cells')
    plt.ylabel('Success Rate')
    plt.savefig(os.path.join(img_dir, f"{exec_file}_success_rate.png"))
    plt.cla()

    for k in range(1, K+1):
        ratio = []
        for m in range(L, R+1, S):
            arr = load_or_run(k, m, N)
            ratio.append(1 - (arr[: , 0].sum() / arr[: , 1].sum()))
        plt.plot(range(L, R+1, S), ratio, label=f'k={k}', marker=markers[k - 1])

    plt.title('Ratio of FlowRadar Decoded Flows on CAIDA')
    plt.legend()
    plt.xlabel('Number of Table Cells')
    plt.ylabel('Success Rate')
    plt.savefig(os.path.join(img_dir, f"{exec_file}_ratio.png"))
    plt.cla()
