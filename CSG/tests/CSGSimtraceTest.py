#!/usr/bin/env python
"""
CSG/tests/CSGSimtraceTest.py
===============================

HMM: almost identical to extg4/tests/X4SimtraceTest.py 

TODO: more duplication avoidance but keep distinct mains

"""
import os, logging, builtins, numpy as np
log = logging.getLogger(__name__)
from opticks.ana.fold import Fold 

import matplotlib.pyplot as mp
from opticks.ana.fold import Fold
from opticks.sysrap.sframe import sframe , X, Y, Z
from opticks.CSG.Values import Values 
from opticks.ana.pvplt import mpplt_simtrace_selection_line, mpplt_hist, mpplt_parallel_lines_auto, mpplt_add_shapes
from opticks.ana.eget import efloatarray_


if __name__ == '__main__':

    #fmt = '[%(asctime)s] p%(process)s {%(pathname)s:%(lineno)d} %(levelname)s - %(message)s'
    fmt = '{%(pathname)s:%(lineno)d} %(levelname)s - %(message)s'
    logging.basicConfig(level=logging.INFO, format=fmt)

    SYMBOLS = os.environ.get("SYMBOLS", None)
    FOLD = os.environ.get("FOLD", None)

    if SYMBOLS is None and not FOLD is None:
        s = Fold.Load(symbol="s")
        t = None
        fr = s.sframe
        s_label = os.environ["GEOM"]
        SYMBOLS = "S" 
        sv = Values.Find("$FOLD", symbol="sv") if not s_label is None else None
    elif not SYMBOLS is None:
        log.info("SYMBOLS defined %s proceed with Fold.MultiLoad", SYMBOLS)
        ff = Fold.MultiLoad()
        log.info(" ff %s " % str(ff))
        assert len(ff) > 0 
        frs = list(filter(None, map(lambda f:f.sframe, ff)))
        assert len(frs) > 0 
        fr = frs[0]       ## HMM: picking first frame, maybe need to form composite bbox from all frames ?
        sv = None
    else:
        assert 0 
    pass

    fig, ax = fr.mp_subplots(mp)  

    if not s is None:
        s_hit = s.simtrace[:,0,3]>0 
        s_pos = s.simtrace[s_hit][:,1,:3]

        ## investigate unexpected top cap intersects : FIXED z2cap TYPO
        ## 
        ##                                                                    intersect pos x > 120.  t > 0 
        ## e = np.logical_and( s.simtrace[:,2,0] > 100., np.logical_and( s.simtrace[:,1,0] > 120. , s.simtrace[:,0,3]>0 )) 
        ## 
        ## e_ori = s.simtrace[e][:100,2,:3]
        ## e_dir = s.simtrace[e][:100,3,:3]
        ## fr.mp_scatter(e_ori, label="e_ori", s=2 ) 
        ## fr.mp_arrow(  e_ori, 10*e_dir, label="e_ori,e_dir", s=2 ) 
    pass

    if s_label.startswith("nmskSolidMaskVirtual") and not sv is None: 
        r1=sv.get("SolidMask.SolidMaskVirtual.rOuter2.mask_radiu_virtual")
        r2=sv.get("SolidMask.SolidMaskVirtual.rOuter3.mask_radiu_virtual/2")
        z1=sv.get("SolidMask.SolidMaskVirtual.zPlane2.htop_out/2")
        z2=sv.get("SolidMask.SolidMaskVirtual.zPlane3.htop_out+MAGIC_virtual_thickness")
        z0=(z2*r1-z1*r2)/(r1-r2)
        ax.set_ylim( -240, z0+50 )
    pass

    mpplt_add_shapes(ax)  # ELLIPSE0 ELLIPSE1 RECTANGLE0 RECTANGLE1


    if not s is None and "UNEXPECTED" in os.environ:  

        if s_label.startswith("nmskSolidMaskVirtual"): 
            w_label, w = "apex glancers",  np.logical_and( np.abs(s.simtrace[:,1,0]) < 220, np.abs(s.simtrace[:,1,2]-98) < 1 ) 
            #w_label, w = "quadratic precision loss", np.logical_and( np.abs(s.simtrace[:,1,0] - (-214)) < 5, np.abs(s.simtrace[:,1,2] - (115)) < 5 )
 
            w_simtrace = s.simtrace[w]
            w_path = "/tmp/simtrace_sample.npy"
            np.save(w_path, w_simtrace)
        else:
            w = np.logical_and( np.abs(s.simtrace[:,1,2]) > 0.20 , s.simtrace[:,0,3]>0 )  
            w_simtrace = s.simtrace[w][::10]
        pass
        log.info("UNEXPECTED w_simtrace : %s " % str(w_simtrace.shape))
        w_ori = w_simtrace[:,2,:3]
        fr.mp_scatter(w_ori, label="w_ori", s=1 )
        mpplt_simtrace_selection_line(ax, w_simtrace, axes=fr.axes, linewidths=2)
    pass

    if not s is None:
        mpplt_parallel_lines_auto( ax, fr.bbox.T, fr.axes, linestyle="dashed" )
    pass
    if not s is None and hasattr(s,"genstep") and "GS" in os.environ:
        s_gs  = s.genstep[:,5,:3]  
        fr.mp_scatter(s_gs, label="s_gs", s=1 )
    pass
 
    if not s is None and hasattr(s,"simtrace_selection"):
       sts = s.simtrace_selection 
    elif not s is None and "SELECTION" in os.environ:
        #w = np.logical_and( s.simtrace[:,0,3]>0, np.logical_and( s.simtrace[:,1,Z] > -38.9, s.simtrace[:,1,Z] < -20. ))
        w = s.simtrace[:,1,X] > 264.5
        wi = np.where(w)[0]  
        sts = s.simtrace[w][:50]
    else:
        sts = None
    pass  

    if not sts is None:
        mpplt_simtrace_selection_line(ax, sts, axes=fr.axes, linewidths=2)
    pass

    d = getattr(s, "CSGDebug_Cylinder", None) if not s is None else None

    log.info("SYMBOLS %s " % str(SYMBOLS))
    if not SYMBOLS is None:
        for A in list(SYMBOLS):
            a = A.lower()
            if hasattr(builtins, a):
                fold = getattr(builtins, a)
                label = getattr(builtins, "%s_label" % a )

            elif a in globals():
                fold = globals()[a]
                label = globals()["%s_label" % a]
            else:
                log.fatal(" a %s not in builtins or globals " % a )
                assert 0 
            pass
            a_offset = efloatarray_("%s_OFFSET" % A, "0,0,0")
            log.info("A %s a %s label %s" % ( A, a, label ) )
            a_hit = fold.simtrace[:,0,3]>0 
            a_pos = a_offset + fold.simtrace[a_hit][:,1,:3]
            label = label.split("__")[0] if "__" in label else label
            fr.mp_scatter(a_pos, label="%s:%s" % (A,label), s=1 )
            pass
        pass
    pass
    fr.mp_legend()
    fig.show()
pass


