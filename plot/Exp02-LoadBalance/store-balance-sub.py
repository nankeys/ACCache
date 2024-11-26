import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.colors import ListedColormap
import seaborn as sns
import numpy as np
import pandas as pd
import os

#sns.set()
#sns.axes_style('white')
colors =["#ee4000", "#5f9ea0", "#9acd32", "#ffa54f"]
mycmap = ListedColormap(sns.color_palette(colors).as_hex())

plt.rcParams['font.sans-serif'] = ['Arial', 'Helvetica']
mpl.rcParams['hatch.linewidth'] = 2
mpl.rcParams["legend.markerscale"] = 1
mpl.rcParams['pdf.fonttype'] = 42

df = pd.read_csv("storagebalance.csv",header=0)
df = df[:4]
print(df)
df = df[['scheme','Cluster23']]
df = df.T
df[1:] = df[1:] * 100
df.columns = df.iloc[0]
df = df.drop(index="scheme")
#df2 = df2.sort_values(by="scheme")

error_params=dict(elinewidth=2,ecolor='black',capsize=3)

fig = plt.figure()
plt.grid(True, zorder=0)
ax = df.plot(kind='bar', rot=0, legend=False, edgecolor='black', lw=2, colormap=mycmap, width = 0.8, zorder=100) #, yerr=0.1, error_kw=error_params)
plt.gcf().set_size_inches(10, 8)
minor_yticks = np.arange(0.015, 0.06, 0.03)
ax.set_yticks(minor_yticks, minor=True)

plt.grid(color="b", linestyle="-", linewidth=0.1, alpha=0.1)
plt.grid(which = 'minor',color="b", linestyle="-", linewidth=0.1, alpha=0.1)


# Hatches in bar plot
patterns =('-', '+', 'x','/','//','O','o','\\','\\\\')
patterns = ['//', '\\\\', "x"]
hatches = [p for p in patterns for i in range(len(df))]
#bars = fig.patches

#for bar, hatch in zip(bars, hatches):
#    bar.set_hatch(hatch)

ax.set_ylim(0, 0.062)
ax.set_yticks(np.arange(0, 0.061, 0.03))

# Hide the top and right axis
# for spine in ['top', 'right']:
#     ax.spines[spine].set_visible(False)

# linewidth of axises, and the fontsize of ticks
plt.setp(ax.spines.values(), linewidth=2)
ax.tick_params(width=2, labelsize = 34)

# ax.set_xlabel("#-th day", fontsize=42)
# ax.set_ylabel("Percent imbalance ($\lambda$)", fontsize=42)

# Legend style
# l = ax.legend(loc='center left', ncols= 4, bbox_to_anchor=(0.1, 0.9), fontsize=36, reverse=False, labelspacing=0, columnspacing=0.5, frameon=False, handlelength=0.8, handleheight=1, handletextpad=0.3)
# for t in l.get_texts(): t.set_position((0, 0))

#plt.show()
plt.gcf().set_size_inches(5, 4)
plt.savefig("storage-balance-sub.pdf", bbox_inches = 'tight', pad_inches = 0)