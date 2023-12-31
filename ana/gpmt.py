#!/usr/bin/env python
"""
::

    ipython -i gpmt.py 


    mkdir -p /Users/blyth/simoncblyth.bitbucket.io/env/presentation/ana/gpmt/
    cp /tmp/fig/*.png /Users/blyth/simoncblyth.bitbucket.io/env/presentation/ana/gpmt/


https://stackoverflow.com/questions/22959698/distance-from-given-point-to-given-ellipse

"""

import os, sys, argparse, logging, textwrap
import numpy as np, math 
import matplotlib.pyplot as plt
import matplotlib.lines as mlines

specs_ = lambda s:filter(lambda s:s[0] != "#", filter(None,map(str.strip, textwrap.dedent(s).split("\n"))))

log = logging.getLogger(__name__)
sys.path.insert(0, os.path.expanduser("~"))  # assumes $HOME/opticks 

from opticks.analytic.GDML import GDML
from opticks.ana.shape import ellipse_points, circle_points
from opticks.ana.gplt import GPlot, add_line
from opticks.ana.gargs import GArgs

from j.PMTEfficiencyCheck_ import PMTEfficiencyCheck_


if __name__ == '__main__':
    args = GArgs.parse(__doc__)
    path = args.gdmlpath(4)    # 3:origin_CGDMLKludge_jun15  4:origin_CGDMLKludge_jun28
    path = os.path.expandvars(path)

    if not os.path.exists(path):
        log.fatal("GDML path does not exist: %s " % path )
        sys.exit(1)
    else:
        log.info("parsing GDML path:%s " % path)
    pass

    g = GDML.parse(path)  
    g.smry()

    NNVT = 1
    HAMA = 2 
    pmt = NNVT 
    #pmt = HAMA 


    pec = None
    #pec = PMTEfficiencyCheck_()
    if not pec is None:
        ipec = { HAMA:0,NNVT:1 }
        pec_sli = 10000
    pass


    lvx = args.lvname(pmt) 
    lv = g.find_one_volume(lvx)

    if lv == None:
        log.fatal("failed to find lvx:[%s] " % (lvx)) 
    assert lv

    #s = lv.solid 
    #s.sub_traverse()

    log.info( "lv %r" % lv )

    lvs = g.get_traversed_volumes( lv, maxdepth=args.maxdepth )


    log.info( "lvs %r" % lvs )
    figpath_suffix  = lv.local_prefix.replace("/","_") 
    log.info( "lv.local_prefix %s" % lv.local_prefix )


    plt.ion()

    fig, ax = GPlot.MakeFig(plt, lv, args, recurse=True)  # all volumes together
    if not pec is None:
        pec.rz_plot(ax, ipec[pmt], pec_sli )
    pass

    ax.set_aspect('equal')
    fig.show()
    path = args.figpath("CombinedFig_%s" % figpath_suffix )
    log.info("saving to %s " % path)
    fig.savefig(path)
    
    #axs = GPlot.MultiFig(plt, lvs, args)

    fig, axs = GPlot.SubPlotsFig(plt, [lvs], args)
    if not pec is None:
        pec.rz_plot(axs, ipec[pmt], pec_sli )
    pass

    fig.show()
    path = args.figpath("SplitFig_%s" % figpath_suffix )
    log.info("saving to %s " % path)
    fig.savefig(path)

    #scribble( axs[0,2] )


def scribble(ax):
    mm = 1. 
    deg = 2.*np.pi/360.

    m4_torus_r = 80. 
    m4_torus_angle = 45.*deg
    m4_r_2 = 254./2.
    m4_r_1 = (m4_r_2+m4_torus_r) - m4_torus_r*np.cos(m4_torus_angle)

    m4_h = m4_torus_r*np.sin(m4_torus_angle) + 5.0       # full height of the tube

    m4_h/2   #    tube to centerline torus offset : so torus centerline level with bottom of tube 

    neck_z = -210.*mm+m4_h/2.
    torus_z = neck_z - m4_h/2 

    torus_x = m4_r_2+m4_torus_r    # radial distance to center of torus circle      

    add_line(ax, [-300,torus_z], [300,torus_z] )       
    add_line(ax, [torus_x, -300], [torus_x, 300] )       
    
    e = ellipse_points( xy=[0,-5.], ex=254., ez=190., n=1000000 )

    #ax.scatter( e[:,0], e[:,1], marker="." )

    tc = np.array([torus_x,torus_z])
    tr = m4_torus_r  
    t = circle_points( xy=tc, tr=tr , n=100 )
    #ax.scatter( t[:,0], t[:,1], marker="." )

    e_inside_t = np.sqrt(np.sum(np.square(e-tc),1)) - tr < 0.  # points on the ellipse that are inside the torus circle 

    ax.scatter( e[e_inside_t][:,0], e[e_inside_t][:,1], marker="." ) 




