# сетка 18000 * 60000

import matplotlib.pyplot as plt
import numpy as np

task1 = [[1001691], [1727581], [1375136], [1169149], [1280056], [1081575]]
# task2 = [[1345022], [321871]]
task3 = [[1563223], [768687], [700433], [474386], [441069], [327138]]

def PrintSpeed (str, arrs):
    n = [1, 2, 3, 4, 5, 6]
    T = []
    for arr in arrs:
        T.append(sum(arr) / len(arr))
    
    T = np.array(T)
    S = T[0] / T
    E = S / n

    plt.rc ('font', size = 13)
    fig = plt.figure(figsize = (16, 8))
    ax1 = fig.add_subplot(1, 2, 1)
    ax1.plot(n, S)
    ax1.set_ylabel("S(n), " + str)
    ax1.grid()

    ax2 = fig.add_subplot(1, 2, 2)
    ax2.plot(n, E)
    ax2.set_ylabel("E(n), " + str)
    ax2.grid()
    fig.savefig(str + "_speed.pdf")

PrintSpeed ("task1", task1)
# PrintSpeed ("task2", task2)
PrintSpeed ("task3", task3)
# PrintSpeed ("with_O3", [t_1_O3, t_2_O3, t_3_O3, t_4_O3, t_5_O3, t_6_O3])
