from arr import *
import matplotlib.pyplot as plt

x = []
t = []
u = []

for elem in arr:
    x.append(elem[0])
    t.append(elem[1])
    u.append(elem[2])

plt.rc ('font', size = 13)
fig = plt.figure(figsize = (10, 10))
ax = fig.add_subplot(projection='3d')
ax.scatter(x, t, u)
plt.savefig("res.pdf")
