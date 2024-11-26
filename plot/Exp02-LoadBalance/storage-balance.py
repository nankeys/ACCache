import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.colors import ListedColormap
import seaborn as sns
import numpy as np
import pandas as pd
import os

#sns.set()
#sns.axes_style('white')
colors =["#ee4000", "#5f9ea0", "#9acd32", "#ffa54f", "#a56cc1"]
mycmap = ListedColormap(sns.color_palette(colors).as_hex())

plt.rcParams['font.sans-serif'] = ['Arial', 'Helvetica']
mpl.rcParams['hatch.linewidth'] = 2
mpl.rcParams["legend.markerscale"] = 1
mpl.rcParams['pdf.fonttype'] = 42

df = pd.read_csv("storagebalance2.csv",header=0)
print(df)
df = df.T
df[1:] = df[1:] * 100

df.columns = df.iloc[0]
df = df.drop(index="scheme")
# df = df[df['scheme' != 'YCSB']]
print(df)
#df2 = df2.sort_values(by="scheme")

error_params=dict(elinewidth=2,ecolor='black',capsize=3)

fig = plt.figure()
plt.grid(True, zorder=0)
ax = df.plot(kind='bar', rot=0, legend=False, edgecolor='black', lw=2, colormap=mycmap, width = 0.8, zorder=100) #, yerr=0.1, error_kw=error_params)
plt.gcf().set_size_inches(10, 8)


# y_axis=df
# for ya in y_axis:
#     for x,y in zip(range(len(df)), df[ya]):
#         if(x != 2): continue
#         print(x,y)
#         if(ya == "FastCache"):
#             x = x - 0.43
#         elif(ya == "EC-Cache"):
#             x = x - 0.15
#         elif(ya == "SP-Cache"):
#             x = x + 0.05
#         elif(ya == "Random"):
#             x = x + 0.25
#         plt.text(x, y+0.5, round(y,2), fontsize=25, rotation=45)


# Hatches in bar plot
patterns =('-', '+', 'x','/','//','O','o','\\','\\\\')
patterns = ['//', '\\\\', "x"]
hatches = [p for p in patterns for i in range(len(df))]

ax.set_ylim(0, 28)
ax.set_yticks(np.arange(0, 28, 5))
minor_yticks = np.arange(5, 28, 5)
ax.set_yticks(minor_yticks, minor=True)

plt.grid(color="b", linestyle="-", linewidth=0.1, alpha=0.1)
plt.grid(which = 'minor',color="b", linestyle="-", linewidth=0.1, alpha=0.1)

# linewidth of axises, and the fontsize of ticks
plt.setp(ax.spines.values(), linewidth=2)
ax.tick_params(width=2, labelsize = 34)

ax.set_xlabel("Selected clusters", fontsize=42)
ax.set_ylabel("Percent imbalance of \n memory overhead (%)", fontsize=38)

# Legend style
l = ax.legend(loc='center left', ncols= 5, bbox_to_anchor=(0.05, 0.93), fontsize=36, reverse=False, labelspacing=0, columnspacing=0.5, frameon=False, handlelength=0.8, handleheight=1, handletextpad=0.3)
for t in l.get_texts(): t.set_position((0, 0))

#plt.show()
plt.gcf().set_size_inches(22, 7)
plt.savefig("storage-balance.pdf", bbox_inches = 'tight', pad_inches = 0)