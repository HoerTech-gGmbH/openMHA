#!/usr/bin/env python
# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 HörTech gGmbH
#
# openMHA is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# openMHA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License, version 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License, 
# version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

from matplotlib import pyplot as plt
import numpy as np

def main():
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)
    plt.ylabel(r'$\mathrm{L_{Out}}\,$/dB',size=14)
    plt.xlabel(r'$\mathrm{L_{In}}\,$/dB',size=14)
    ax.set_ylim([0,110])
    ax.set_xlim([0,110])

    x1=[0,20, 20, 60, 100, 110 ]
    y1=[0, 0 , 40, 90, 90, 90 ]
    x2=[x1[0],x1[-1]]
    y2=x2
    plt.plot(x1,y1)
    plt.plot(x2,y2,color='black')
    plt.vlines([60, 100], 0, 110,linestyles='dotted')
    plt.vlines([20], 0, 110,linestyles='dotted')
    plt.hlines([60, 90], 50, 60,linestyles='dotted')
    plt.hlines([20, 40], 10, 20,linestyles='dotted')
    plt.hlines([90, 100], 90, 100,linestyles='dotted')
    plt.annotate("",xy=(0,60), arrowprops=dict(arrowstyle='<->'), xytext=(20, 60))
    plt.annotate("",xy=(20,50), arrowprops=dict(arrowstyle='<->'), xytext=(60, 50))
    plt.annotate("",xy=(60,40), arrowprops=dict(arrowstyle='<->'), xytext=(100, 40))

    plt.annotate("gtmin",xy=(10,60),ha='center',va='bottom')
    plt.annotate("gstep",xy=(80,40),ha='center',va='bottom')
    plt.annotate("gtstep",xy=(40,50),ha='center',va='bottom')



    plt.annotate("",xy=(10,20), arrowprops=dict(arrowstyle='<->'),horizontalalignment='right',verticalalignment='bottom', xytext=(10,40))
    plt.annotate("",xy=(50,60), arrowprops=dict(arrowstyle='<->'),horizontalalignment='left',verticalalignment='bottom',xytext=(50,90))
    plt.annotate("",xy=(90,100), arrowprops=dict(arrowstyle='<->'),horizontalalignment='left',verticalalignment='bottom', xytext=(90,90))
    plt.annotate("gt[0]",xy=(10,30))
    plt.annotate("gt[1]",xy=(50,75))
    plt.annotate("gt[2]",xy=(80,95))
    plt.tight_layout()
    plt.savefig("dc_in_out.png")
    plt.show()


with plt.style.context(u'seaborn-paper'):
    main()

