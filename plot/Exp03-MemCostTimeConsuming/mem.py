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
colors =["#DC143C", "#9acd32", "#ffa54f", "#87CEFA"]
mycmap = ListedColormap(sns.color_palette(colors).as_hex())

plt.rcParams['font.sans-serif'] = ['Arial', 'Helvetica']
mpl.rcParams['hatch.linewidth'] = 2
mpl.rcParams["legend.markerscale"] = 1
mpl.rcParams['pdf.fonttype'] = 42

df = pd.read_csv("data_mem.csv",header=0)
print(df)
df = df.T
#df = df.dropna()
df[1:] = df[1:] / 1024
df.columns = df.iloc[0]
df = df.drop(index="scheme")
#df2 = df2.sort_values(by="scheme")

markers = ["h", "s", MarkerDefine.hexagram, MarkerDefine.rhombus]
dashes = [(1, 0, 1, 0), (2, 2, 2, 2), (5, 2, 5, 2), (5, 1, 2, 1)]

# df_error=np.array([[[0,0,3333.33333333326,3333.33333333326,0], [0,0,6666.66666666674,6666.66666666674,0]],
# [[430.666666666628,430.666666666628,430.666666666628,430.666666666628,430.666666666628], [845.333333333372,845.333333333372,845.333333333372,845.333333333372,845.333333333372]],
# [[639.666666666628,639.666666666628,639.666666666628,639.666666666628,639.666666666628], [735.333333333372,735.333333333372,735.333333333372,735.333333333372,735.333333333372]],
# [[971.333333333372,971.333333333372,971.333333333372,971.333333333372,971.333333333372], [740.666666666628,740.666666666628,740.666666666628,740.666666666628,740.666666666628]]])


fig = plt.figure(figsize=(10, 8.5))
# ax = df.plot(yerr = df_error/1000000, fmt='.', color="black", elinewidth=2, ecolor='black', capsize=5, legend = False, zorder=150)
#reset color cycle so that the marker colors match
#ax.set_prop_cycle(None)
#plot the markers
ax = df.plot(kind='bar', rot=0, lw=6, colormap=mycmap) #, markerfacecolor='none', markersize = 30, mew = 5)
plt.gcf().set_size_inches(10, 8)

# for i, line in enumerate(ax.get_lines()):
#         line.set_marker(markers[i%4])
        #line.set_dashes(dashes[i%4])


# Hatches in bar plot
patterns =('-', '+', 'x','/','//','O','o','\\','\\\\')
patterns = ['//', '\\\\', "x"]
hatches = [p for p in patterns for i in range(len(df))]
bars = ax.patches

#for bar, hatch in zip(bars, markers):
#    bar.set_hatch(hatch)

ax.set_ylim(0, 200)
ax.set_yticks(np.arange(0, 201, 50))

# Hide the top and right axis
# for spine in ['top', 'right']:
#     ax.spines[spine].set_visible(False)

minor_yticks = np.arange(25, 201, 50)
ax.set_yticks(minor_yticks, minor=True)
# ax.set_xticks(np.arange(0, 5, 1), labels=['10','30', '50','70', '100'])
ax.set_xticks([0, 1, 2, 3], labels=["cluster01", "cluster02", "cluster23", "cluster25"], rotation=20)

# y_axis=df
# for ya in y_axis:
#     for x,y in zip(range(len(df)), df[ya]):
#         # if(x != 2): continue
#         print(x,y)
#         if(ya == "CMSketch"):
#             x = x - 0.32
#         elif(ya == "w/o CMSketch"):
#             x = x - 0.03
#         plt.text(x, y+3, round(y,2), fontsize=20, rotation=0)

plt.grid(True)
plt.grid(color="b", linestyle="-", linewidth=0.1, alpha=0.1)
plt.grid(which = 'minor',color="b", linestyle="-", linewidth=0.1, alpha=0.1)

# linewidth of axises, and the fontsize of ticks
plt.setp(ax.spines.values(), linewidth=2.5)
ax.tick_params(width=2, labelsize = 34)

ax.set_xlabel("Selected clusters", fontsize=42)
ax.set_ylabel("Memory overhead (GB)", fontsize=38)

# Legend style
a = ax.legend().get_texts()
label = [x.get_text() for x in a]
l = ax.legend(labels= label[:4], loc='center left', ncols= 1, bbox_to_anchor=(0.00, 0.85), fontsize=36, reverse=False, labelspacing=0.3, columnspacing=0.5, frameon=False, handlelength=0.8, handleheight=1, markerscale=0.8, handletextpad=0.3)
for t in l.get_texts(): t.set_position((0, 2))

plt.gcf().set_size_inches(10, 8.4)
plt.savefig("memory-cost.pdf", bbox_inches = 'tight', pad_inches = 0)