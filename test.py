#! /usr/bin/python

import numpy as np
import matplotlib.pyplot as plt

x = np.arange(0, 5, 0.05);
y = np.sin(x)
print x, y
plt.plot(x, y, linewidth=2.5, linestyle="-", marker="o", markersize=20, alpha=0.75, color="red", markeredgecolor="none")
plt.show()
