import os, sys
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

exec_dir = "./evaluations"
exec_file = "flowradar_decoding_eval"
exec_path = os.path.join(exec_dir, exec_file)

csv_dir = "./data/evaluations/"
img_dir = "./img"

K = 3
L = 100
R = 2000
S = 100
N = 500

def get_name(k=K, l=L, r=R, s=S, n=N):
    return f"{exec_file}_k={k}_l={l}_r={r}_s={s}_n={n}"

def load_or_run(k=K, l=L, r=R, s=S, n=N):
    csv_file = get_name(k, l, r, s, n) + '.csv'
    csv_path = os.path.join(csv_dir, csv_file)
    if not os.path.exists(csv_path):
        cmd = f"{exec_path} -o {csv_path} -k {k} -l {l} -r {r} -s {s} -n {n}"
        os.system(cmd)
    return np.loadtxt(csv_path, dtype='int32', delimiter=',')
    

if __name__ == '__main__':
    os.makedirs(exec_dir, exist_ok=True)
    os.makedirs(csv_dir, exist_ok=True)
    os.makedirs(img_dir, exist_ok=True)

    markers = ['o', 's', '^', 'p', 'D', 'v', 'h']
    for k in range(1, 4):
        arr = load_or_run(k=k, n=500)
        mean = arr.mean(axis=1)
        plt.plot(range(L, R+1, S), mean, label=f'k={k}', marker=markers[k - 1])
    plt.title('Decoding Eval of FlowRadar')
    plt.legend()
    plt.xlabel('Number of Table Cells')
    plt.ylabel('Average decoded flows')
    plt.savefig(os.path.join(img_dir, f"{exec_file}_count.png"))
    plt.cla()

    
    
