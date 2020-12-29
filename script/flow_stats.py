import os, sys
import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import (MultipleLocator, AutoMinorLocator)

DATA_FILE_NAME = './tmp/tmp_data.npy'

def get_data_from_file():
    print('Read data from file')
    return np.load(DATA_FILE_NAME)

def get_data_by_calc():
    print('Calc data')
    MAX_FLOW = math.inf
    # MAX_FLOW = 1000
    path = './data/caida/flow.bin'
    dic = {}
    with open(path, 'rb') as f:
        while True:
            _epoch_id = int.from_bytes(f.read(8), 'little')
            cnt = int.from_bytes(f.read(8), 'little')
            if cnt == 0 or len(dic) > MAX_FLOW:
                break
            for _ in range(cnt):
                key = f.read(16)[:13]
                # print(key)
                value = (int.from_bytes(f.read(4), 'little'), int.from_bytes(f.read(4), 'little'))
                if key not in dic:
                    dic[key] = []
                dic[key].append(value)
    y = sorted([len(v) for v in dic.values()])
    y = np.asarray(y)
    np.save(DATA_FILE_NAME, y)
    return y

def get_data():
    if os.path.exists(DATA_FILE_NAME):
        return get_data_from_file()
    else:
        return get_data_by_calc()

x = get_data()  # sorted frequency for every flow
y = np.arange(0.0, 1.0, 1.0/len(x))

fig, ax = plt.subplots()
plt.xlabel('flow frequency')
plt.ylabel('CDF')

plt.yticks(np.arange(0.0, 1.01, 0.1))
# ax.yaxis.set_minor_locator(AutoMinorLocator())
plt.xscale('log')
# plt.xlim(0.9, 2 * 10**(math.ceil(math.log10(y[-1]))))
plt.plot(x, y)
# plt.show()
plt.savefig('img/flow_frequency_distribution.png')
plt.cla()

plt.xlabel('flow frequency')
plt.ylabel('CDF')
plt.xscale('log')
y = np.cumsum(x)
z = y / y[-1]
plt.yticks(np.arange(0.0, 1.01, 0.1))
# ax.yaxis.set_minor_locator(AutoMinorLocator())

plt.plot(x, z)
# plt.show()
plt.savefig('img/flow_cnt_frequency_distribution.png')

print(len(x))
print(y[-1])
