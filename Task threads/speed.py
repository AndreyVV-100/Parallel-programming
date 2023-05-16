# eps = 1e-8

import matplotlib.pyplot as plt
import numpy as np

# Без -O3
t_1 = [9374015, 9611205, 9415748, 9450359, 9519654]
t_2 = [8557048, 9111871, 9466056, 7946970, 7864718]
t_3 = [5874929, 5699894, 5888085, 5931200, 5806708]
t_4 = [4865132, 5978267, 4967788, 5986717, 9431088]
t_5 = [4687482, 5776156, 4761634, 4703038, 4584315]
t_6 = [4798089, 4532855, 4756278, 4556010, 4652446]


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

PrintSpeed ("without_O3", [t_1, t_2, t_3, t_4, t_5, t_6])
