import pandas as pd
import seaborn as sns
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
import numpy as np
import os

plt.rcParams['font.sans-serif'] = ['Arial', 'Helvetica']
mpl.rcParams['hatch.linewidth'] = 2
mpl.rcParams["legend.markerscale"] = 1
mpl.rcParams['pdf.fonttype'] = 42

colors =["#ee4000", "#5f9ea0", "#9acd32", "#ffa54f", "#a56cc1"]
mycmap = ListedColormap(sns.color_palette(colors).as_hex())

# Convert the data to a DataFrame
df = pd.read_csv("data_n15.csv", header=0)
df['p95'] *= 1000

# Plotting
# plt.figure(figsize=(12, 8))

# Define the order of the Scheme categories
scheme_order = ['AC-Cache','EC-Cache', 'SP-Cache', 'Baseline',  'Replication']

# Reorder the 'Scheme' column based on the defined order
df['Scheme'] = pd.Categorical(df['Scheme'], categories=scheme_order, ordered=True)

# Group by 'Scheme' and 'NodeNum' and calculate mean and std thruput
grouped = df.groupby(['Scheme', 'NodeNum'])['p95'].agg(['mean', 'std'])

# Reshape the DataFrame for plotting
grouped = grouped.unstack(level='Scheme')

# Plotting with error bars for each scheme
ax = grouped['mean'].plot(kind='bar', yerr=grouped['std'], capsize=5, rot=0, legend=False, edgecolor='black', lw=2, colormap=mycmap, width = 0.8, zorder = 100)

minor_yticks = np.arange(0.05, 0.4, 0.1)

ax.set_ylim(0, 0.43)
ax.set_yticks(np.arange(0, 0.41, 0.1))
ax.set_yticks(minor_yticks, minor=True)

# Hide the top and right axis
# for spine in ['top', 'right']:
#     ax.spines[spine].set_visible(False)
plt.grid(True)

#axes.grid(which = 'minor', alp)

plt.grid(color="b", linestyle="-", linewidth=0.1, alpha=0.1)
plt.grid(which = 'minor',color="b", linestyle="-", linewidth=0.1, alpha=0.1)


# linewidth of axises, and the fontsize of ticks
plt.setp(ax.spines.values(), linewidth=2.5)
ax.tick_params(width=2, labelsize = 34)

ax.set_xlabel("# of caching nodes", fontsize=42)
ax.set_ylabel("Tail latency (ms)", fontsize=42)

# Legend style
a = ax.legend().get_texts()
label = [x.get_text() for x in a]
l = ax.legend(labels= label[:5],loc='center left', ncols= 2, bbox_to_anchor=(0.12, 0.83), fontsize=36, reverse=False, labelspacing=0.3, columnspacing=0.5, frameon=False, handlelength=1, handleheight=1.3, markerscale=0.8, handletextpad=0.3)
for t in l.get_texts(): t.set_position((0, 6))

plt.gcf().set_size_inches(10, 8.5)
plt.savefig("tail-latency.pdf", bbox_inches = 'tight', pad_inches = 0)