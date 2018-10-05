#!/usr/bin/env python
from matplotlib import pyplot as plt
import numpy as np
from math import atan2,atan



def main():
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    #ax.spines['right'].set_color('none')
    #ax.spines['top'].set_color('none')
    plt.xticks([20,50,80])
    plt.yticks([50, 70, 80, 90, 100])
    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)

    ax.set_yticklabels(['50', '', '80', '', 'MPO'])
    ax.set_xticklabels(['noise gate','50','80'])
    plt.ylabel(r'$\mathrm{L_{Out}}\,$/dB',size=14)
    plt.xlabel(r'$\mathrm{L_{In}}\,$/dB',size=14)
    ax.set_ylim([0,110])
    ax.set_xlim([0,110])
    x0=[0,20]
    y0=[110/3, 50]
    x1=[15, 20, 50, 80,95, 120 ]
    y1=[0, 50, 70, 90, 100, 100 ]
    x2=[0,x1[-1]]
    y2=x2

    plt.vlines([50], 0, 70,linestyles='dotted')
    plt.vlines([80], 0, 90,linestyles='dotted')
    plt.hlines([50], 0, 50,linestyles='dotted')
    plt.hlines([70], 0, 50,linestyles='dotted')
    plt.hlines([80], 0, 80,linestyles='dotted')
    plt.hlines([90], 0, 80,linestyles='dotted')
    plt.hlines([100], 0, 120,linestyles='dotted')
    plt.vlines([20], 0, 50,linestyles='dotted')
    points=np.array((15,25)).reshape((1,2))
    trans_angle=plt.gca().transData.transform_angles(np.array((atan2(50,5)*180/3.1415,)),points)[0]
    plt.annotate("expansion", xy=(19.5,45),va='top',ha='right',rotation=trans_angle)

    plt.annotate("", xy=(15,50), xytext=(15,70),arrowprops=dict(arrowstyle='<->'))
    plt.annotate("G50",xy=(14,60),va='center',ha='right')

    plt.annotate("", xy=(25,80), xytext=(25,90),arrowprops=dict(arrowstyle='<->'))
    plt.annotate("G80",xy=(24,85),va='center',ha='right')
    plt.plot(x0,y0,linestyle='dotted',color='black')
    plt.plot(x1,y1)
    plt.plot(x2,y2,color='black')
    plt.tight_layout()
    plt.savefig("dc_simple_in_out.png")
    plt.show()


#with plt.style.context(('seaborn-paper')):
with plt.style.context(u'seaborn-paper'):
    main()

