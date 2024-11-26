import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.colors import ListedColormap
import seaborn as sns
import numpy as np
import pandas as pd
import os

import MarkerDefine

#sns.set()
#sns.axes_style('white')
colors =["#EB455F", "#F2921D", "#068FFF", "#609966"]
mycmap = ListedColormap(sns.color_palette(colors).as_hex())

plt.rcParams['font.sans-serif'] = ['Arial', 'Helvetica']
mpl.rcParams['hatch.linewidth'] = 2
mpl.rcParams["legend.markerscale"] = 1
mpl.rcParams['pdf.fonttype'] = 42

df = pd.read_csv("data_l95.csv",header=0)
print(df)
df = df.T
# df[1:] = df[1:] * 1000
df.columns = df.iloc[0]
df = df.drop(index="traceno")
#df2 = df2.sort_values(by="scheme")

markers = ["h", "s", MarkerDefine.hexagram, MarkerDefine.rhombus]
dashes = [(1, 0, 1, 0), (2, 2, 2, 2), (5, 2, 5, 2), (5, 1, 2, 1)]

# df_error=np.array([[[1.33E-06,6.67E-07,6.67E-07,6.67E-07,3.33E-07], [6.67E-07,1.33E-06,3.33E-07,3.33E-07,6.67E-07]],
# [[6.67E-07,6.67E-07,6.67E-07,6.67E-07,6.67E-07], [3.33E-07,3.33E-07,3.33E-07,3.33E-07,3.33E-07]],
# [[3.33E-07,3.33E-07,3.33E-07,3.33E-07,3.33E-07], [6.67E-07,6.67E-07,6.67E-07,6.67E-07,6.67E-07]],
# [[1.67E-06,1.67E-06,1.67E-06,1.67E-06,1.67E-06], [1.33E-06,1.33E-06,1.33E-06,1.33E-06,1.33E-06]]])

fig = plt.figure()
# ax = df.plot(yerr = df_error/1000000, fmt='.', color="black", elinewidth=2, ecolor='black', capsize=5, legend = False, zorder=150)
#reset color cycle so that the marker colors match
# ax.set_prop_cycle(None)
#plot the markers
ax = df.plot(kind='line', rot=0, lw=6, colormap=mycmap, markerfacecolor='none', markersize = 30, mew = 5)
plt.gcf().set_size_inches(10, 8)

for i, line in enumerate(ax.get_lines()):
    #print(i)
    # if(11< i < 16):
    line.set_marker(markers[i%4])
        # line.set_dashes(dashes[i%4])

# Hatches in bar plot
patterns =('-', '+', 'x','/','//','O','o','\\','\\\\')
patterns = ['//', '\\\\', "x"]
hatches = [p for p in patterns for i in range(len(df))]
bars = ax.patches

#for bar, hatch in zip(bars, markers):
#    bar.set_hatch(hatch)

ax.set_ylim(100, 150)
# ax.set_yticks([0, 0.05, 0.1, 0.15, 0.2])

minor_yticks = np.arange(100, 151, 10)
ax.set_yticks(minor_yticks, minor=True)

plt.grid(True)
plt.grid(color="b", linestyle="-", linewidth=0.1, alpha=0.1)
plt.grid(which = 'minor',color="b", linestyle="-", linewidth=0.1, alpha=0.1)

# Hide the top and right axis
# for spine in ['top', 'right']:
#     ax.spines[spine].set_visible(False)

# linewidth of axises, and the fontsize of ticks
plt.setp(ax.spines.values(), linewidth=2.5)
ax.tick_params(width=2, labelsize = 34)

ax.set_xlabel(r"Correlation threshold", fontsize=42)
ax.set_ylabel(r"Tail latency ($\mu$s)", fontsize=42)

# Legend style
a = ax.legend().get_texts()
label = [x.get_text() for x in a]
l = ax.legend(labels= label[:4], loc='center left', ncols= 2, bbox_to_anchor=(0.05, 0.9), fontsize=36, reverse=False, labelspacing=0.3, columnspacing=0.5, frameon=False, handlelength=0.8, handleheight=1, markerscale=0.8, handletextpad=0.3)
for t in l.get_texts(): t.set_position((0, 2))

plt.gcf().set_size_inches(10, 8.3)
plt.savefig("tail-latency.pdf", bbox_inches = 'tight', pad_inches = 0)