from res import *
import matplotlib.pyplot as plt
import numpy as np

params = [param0, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10]
x = np.linspace(0, 1, len(param0))

plt.rc ('font', size = 13)
fig = plt.figure(figsize = (10, 10))
ax = fig.add_subplot()

for graph in params:
    ax.plot(x, graph)


plt.savefig("res.pdf")
