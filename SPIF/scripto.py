

import os
import numpy as np
import pdb


t_array = np.concatenate((np.logspace(4,1, num=200, base=10), np.array([1])))


# pdb.set_trace()

print("t\t count")
for i in t_array:
    # os.system(f"./sender 32 32 {i} 10.37.222.2:3331")
    os.system(f"./sender 32 32 {i} 172.16.223.106:3333")
