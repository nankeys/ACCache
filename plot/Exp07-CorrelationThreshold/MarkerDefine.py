import matplotlib.path as mpath
import matplotlib.pyplot as plt
import numpy as np

text_style = dict(horizontalalignment='right',                      verticalalignment='center',
                  fontsize=12, fontfamily='monospace')
marker_style = dict(linestyle=':', color='0.8', markersize=10,
                    markerfacecolor="none", markeredgecolor="tab:red")


def format_axes(ax):
    ax.margins(0.2)
    ax.set_axis_off()
    ax.invert_yaxis()


def split_list(a_list):
    i_half = len(a_list) // 2
    return a_list[:i_half], a_list[i_half:]

sverts = [
    (-1., 1.),
    (1.,1.),
    (1.,-1.),
    (-1.,-1.),
    (-1., 1.),
]
rverts = [
    (0,1),
    (1,0),
    (0,-1),
    (-1,0),
    (0,1),
]
qcodes = [
    mpath.Path.MOVETO,
    mpath.Path.LINETO,
    mpath.Path.LINETO,
    mpath.Path.LINETO,
    mpath.Path.CLOSEPOLY,
]

cross = mpath.Path.unit_regular_asterisk(4)
circle = mpath.Path.unit_circle()

square = mpath.Path(sverts, qcodes)
rhombus = mpath.Path(rverts, qcodes)

ocross = mpath.Path(
    vertices=np.concatenate([circle.vertices, cross.vertices[::-1, ...]]),
    codes=np.concatenate([circle.codes, cross.codes]))

csquare = mpath.Path(
    vertices=np.concatenate([square.vertices, cross.vertices[::-1, ...]]),
    codes=np.concatenate([square.codes, cross.codes]))

crhombus = mpath.Path(
    vertices=np.concatenate([rhombus.vertices, cross.vertices[::-1, ...]]),
    codes=np.concatenate([rhombus.codes, cross.codes]))

ploygon = mpath.Path.unit_regular_polygon(6)
#hexagram = mpath.Path.unit_regular_star(6)

#print(ploygon.vertices, hexagram.vertices)

tverts = [
    [-8.66025404e-01,5.00000000e-01],
    [ 8.66025404e-01,5.00000000e-01],
    [-1.83697020e-16,-1.00000000e+00],
    [-8.66025404e-01,5.00000000e-01],
    [-8.66025404e-01,-5.00000000e-01],
    [ 8.66025404e-01, -5.00000000e-01],
    [ 6.12323400e-17,  1.00000000e+00],
    [-8.66025404e-01, -5.00000000e-01],
]

tcodes = [
    mpath.Path.MOVETO,
    mpath.Path.LINETO,
    mpath.Path.LINETO,
    mpath.Path.CLOSEPOLY,
    mpath.Path.MOVETO,
    mpath.Path.LINETO,
    mpath.Path.LINETO,
    mpath.Path.CLOSEPOLY,
]

hexagram = mpath.Path(tverts, tcodes)

markers = {'square':square, "rhombus": rhombus, 'csquare':csquare, 'crhombus':crhombus, 'ploygon': ploygon,'hexagram':hexagram}

if __name__ == "__main__":
    fig, ax = plt.subplots()
    fig.suptitle('Path markers', fontsize=14)
    fig.subplots_adjust(left=0.4)
    for y, (name, marker) in enumerate(markers.items()):
        ax.text(-0.5, y, name, **text_style)
        ax.plot([y] * 3, marker=marker, **marker_style)
    format_axes(ax)

    plt.show()