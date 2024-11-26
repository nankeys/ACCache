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
mpl.rcParams["legend.markerscale"] = 5
mpl.rcParams['pdf.fonttype'] = 42

# Convert the data to a DataFrame
df = pd.read_csv("data.csv", header=0)
df['ops'] /= 1000000

scheme_order = ['AC-Cache','EC-Cache', 'SP-Cache', 'Baseline',  'Replication']

# Reorder the 'Scheme' column based on the defined order
df['Scheme'] = pd.Categorical(df['Scheme'], categories=scheme_order, ordered=True)

# Group by 'Scheme' and 'NodeNum' and calculate mean and std thruput
grouped = df.groupby(['Scheme', 'ClusterNum'])['ops'].agg(['mean', 'std'])

# Reshape the DataFrame for plotting
grouped = grouped.unstack(level='Scheme')

# Plotting with error bars for each scheme
ax = grouped['mean'].plot(kind='bar', yerr=grouped['std'], capsize=5, rot=0, legend=False, edgecolor='black', lw=2, colormap=mycmap, width = 0.8, zorder = 100)


ax.set_ylim(0, 5.5)
ax.set_yticks(np.arange(0, 5.1, 1))

minor_yticks = np.arange(0.5, 5, 1)
ax.set_yticks(minor_yticks, minor=True)
plt.grid(True, zorder=0)
plt.grid(color="b", linestyle="-", linewidth=0.1, alpha=0.1)
plt.grid(which = 'minor',color="b", linestyle="-", linewidth=0.1, alpha=0.1)

ax.set_xticks([0, 1, 2, 3, 4, 5], labels=["cluster01", "cluster02", "cluster23", "cluster25", '202206', '202401'], rotation=30)

#ax.set_xticks(["""1""","""2""","""23""","""34"""],labels=[1, 2, 23, 34])

# Hide the top and right axis
# for spine in ['top', 'right']:
#     ax.spines[spine].set_visible(False)

# linewidth of axises, and the fontsize of ticks
plt.setp(ax.spines.values(), linewidth=2)
ax.tick_params(width=2, labelsize = 34)

ax.set_xlabel(r"Selected clusters", fontsize=42)
ax.set_ylabel("Throughput (Mops)", fontsize=42)

# Legend style
ax.legend(loc='center left', ncols= 2, bbox_to_anchor=(0.05, 0.83), fontsize=36, reverse=False, labelspacing=0.3, columnspacing=0.5, frameon=False, handlelength=0.8, handleheight=1, handletextpad=0.3)

#plt.show()
plt.gcf().set_size_inches(10, 8)
plt.savefig("iops.pdf", bbox_inches = 'tight', pad_inches = 0)