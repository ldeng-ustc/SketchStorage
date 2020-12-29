import os, sys
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
img_dir = "./img/"
csv_path = "./data/evaluations/leveldb/delay_Write.csv"
arr = np.loadtxt(csv_path, dtype='int32', delimiter=',')
srt = np.sort(arr)
plt.yscale('log')
plt.xlabel('Percent')
plt.ylabel('Delay (us)')
plt.plot(np.arange(0, 100, 0.01), srt)
plt.savefig(os.path.join(img_dir, "leveldb_write_delay.png"))

print("mean:", np.mean(srt))
print("90p:", srt[9000])
print("95p:", srt[9500])


