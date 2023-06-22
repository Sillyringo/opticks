#!/usr/bin/env python
import numpy as np
from opticks.ana.fold import Fold

import matplotlib.pyplot as plt
SIZE = np.array([1280,720])

if __name__ == '__main__':
    t = Fold.Load(symbol="t")
    print(repr(t))
    a = t.test_dot_pol_cross_mom_nrm     

    title = "test_dot_pol_cross_mom_nrm"
    fig, ax = plt.subplots(1, figsize=SIZE/100.)
    fig.suptitle(title)

    ax.scatter(  a[:,0], a[:,1], label="a[:,0] [:,1]  fr, pot " )
    ax.scatter(  a[:,0], a[:,2], label="a[:,0] a[:,2] fr, pot/mct " )
    ax.scatter(  a[:,0], a[:,3], label="a[:,0] a[:,3] fr, pot/st " )

    ax.plot( [0,1], [-1,-1],  label="-1" )
    ax.plot( [0,1], [+1,+1],  label="+1" )

    ax.legend()
    fig.show()

pass


