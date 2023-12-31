#!/usr/bin/env python
#
# Copyright (c) 2019 Opticks Team. All Rights Reserved.
#
# This file is part of Opticks
# (see https://bitbucket.org/simoncblyth/opticks).
#
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.  
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
# See the License for the specific language governing permissions and 
# limitations under the License.
#

"""
prim.py 
=========

Loads a list of primitives from a GParts persisted directory.
Used for debugging things such as missing transforms.

See: notes/issues/OKX4Test_partBuffer_difference.rst

TODO: consolidate ana/prim.py with dev/csg/GParts.py 

"""
import sys, os, logging, numpy as np
log = logging.getLogger(__name__)
from opticks.sysrap.OpticksCSG import CSG_
from opticks.ana.blib import BLib
from opticks.ana.mesh import Mesh

from opticks.ana.shape import Shape

class Ellipsoid(Shape):pass
class Tubs(Shape):pass
class Torus(Shape):pass
class Cons(Shape):pass
class Hype(Shape):pass
class Box(Shape):pass

class UnionSolid(Shape):pass
class SubtractionSolid(Shape):pass
class IntersectionSolid(Shape):pass


class Part(object):
    """
    Parts are CSG constituents, aka nodes of the CSG trees that make up each solid  
    """
    part_idx = 0 

    def __init__(self, part, trans, d, prim):
        """
        Part instances are created within the parent Prim instance
        by mapping this Part ctor over elements of the parts (nodes)
        and global transforms arrays.

        :param part: single csg node of shape (4,4)
        :param trans: 1 or more transforms, shape (ntran,4,4)
        :param d: Dir instance 
        """

        sidx = d.sidx
        self.d = d 
        self.prim = prim 
        self.sidx = sidx

        assert part.shape == (4,4), part
        assert trans.shape[1:] == (4,4), trans
        assert trans.shape[0] < 16, trans
        assert d.__class__.__name__ == 'Solid'   # formerly 'Dir'
        ntran = trans.shape[0]
        assert ntran > 0

        log.debug( "Part : trans.shape %s " % repr(trans.shape))
        #print("trans", trans) 

        f = part.view(np.float32) 
        u = part.view(np.uint32)

        fc = part.copy().view(np.float32)
        fc[1][2] = 0  # scrub boundary in copy, as it is known discrepant : due to lack of surfaces


        tc = u[2][3]          ## typecode eg CSG_.UNION
        tcn = CSG_.desc(tc)   ## typename 
        bnd = u[1][2]         ## boundary index

        # check the complements, viewing as float otherwise lost "-0. ok but not -0"
        comp = np.signbit(f[3,3])  
         # shift everything away, leaving just the signbit 
        comp2 = u[3,3] >> 31
        assert comp == comp2 

        # recover the gtransform index by getting rid of the complement signbit  
        gt = u[3][3] & 0x7fffffff

        if tc in [CSG_.DIFFERENCE, CSG_.INTERSECTION, CSG_.UNION]:
            assert gt == 0, "operators are expected to not have a gtransform" 
        elif tc == CSG_.ZERO:
            assert gt == 0, "zeros are not expected to have a gtransform"
        else:
            assert gt > 0, "primitives are expected to have a gtransform "   
            
            #log.info(" sidx:%2d part_idx:%5d ntran:%2d gt:%2d tcn:%s " % ( sidx, self.__class__.part_idx, ntran, gt, tcn )) 

            if not gt <= ntran:
                log.warn("1-based gt expected to be <= ntran (local index, not global)  gt:%d ntran:%d " % (gt,ntran)) 
            pass
            #assert gt <= ntran
            if ntran > 5:
                pass
                #log.info(" part_idx:%5d ntran:%2d gt:%2d tcn:%s " % ( self.__class__.part_idx, ntran, gt, tcn )) 
            pass
        pass


        self.f = f
        self.fc = fc

        self.tc = tc
        self.tcn = tcn 
        self.comp = comp
        self.gt = gt        # 1-based gtransform pointer into trans

        self.bnd = bnd
        self.bname = d.blib.bname(bnd)

        try:
            tran = trans[gt-1] if gt > 0 else np.eye(4)
        except IndexError:
            log.error("trans issue gt %s trans.shape %s " % ( gt, repr(trans.shape)))
            tran = np.eye(4)
        pass

        self.tran = tran 

        self.idx = self.__class__.part_idx 
        self.__class__.part_idx += 1 



    def __repr__(self):
        return "    Part %1s%2s %2s  %15s   %3d %25s   tz:%10.3f    %s  " % ( "!" if self.comp else " ", self.tc, self.gt, self.tcn, self.bnd, self.bname, self.tz, self.detail() )

    def maxdiff(self, other):
        """
        :param other: Part instance
        :return float: max absolute difference between float param values of the CSG constituent part
        (actually the boundary index is excluded by comparing a copy and scrubbing that) 
        """
        return np.max( np.abs(self.fc - other.fc) )

    r = property(lambda self:self.f[0][3])
    tz = property(lambda self:self.tran[3][2])

    xbox = property(lambda self:self.f[0][0])
    ybox = property(lambda self:self.f[0][1])
    zbox = property(lambda self:self.f[0][2])

    r1co = property(lambda self:self.f[0][0])
    z1co = property(lambda self:self.f[0][1])
    r2co = property(lambda self:self.f[0][2])
    z2co = property(lambda self:self.f[0][3])

    z1 = property(lambda self:self.f[1][0])  # cy or zsphere
    z2 = property(lambda self:self.f[1][1])
    r1 = property(lambda self:self.f[0][2])
    r2 = property(lambda self:self.f[0][3])
    dz = property(lambda self:self.z2 - self.z1)
    dr = property(lambda self:self.r2 - self.r1)


    def as_shape(self, name, sc):
        if self.tc == CSG_.BOX3:
            sh = Box(name, [self.xbox/sc, self.zbox/sc ] )
        elif self.tc == CSG_.CYLINDER:
            sh = Tubs(name, [self.r/sc, abs(self.z1/sc) ] )
        elif self.tc == CSG_.TORUS:
            sh = Ellipsoid(name, [self.r/sc, self.r/sc ] )
        else:
            sh = None
        pass
        if not sh is None:
            sh.ltransform = [0, self.tz/sc ]  
        pass
        return sh 

    def detail(self):
        tz = self.tz
        if self.tc == CSG_.ZSPHERE:
            msg = " r: %10.3f z1:%10.3f z2:%10.3f " % ( self.r, self.z1, self.z2 ) 
        elif self.tc == CSG_.SPHERE:
            msg = " r: %10.3f " % ( self.f[0][3]  ) 
        elif self.tc == CSG_.CYLINDER:
            msg = "   z1:%10.3f z2:%10.3f r :%10.3f               z1+tz:%10.3f z2+tz:%10.3f" % ( self.z1, self.z2, self.r, self.z1 + tz, self.z2 + tz) 
        elif self.tc == CSG_.CONE:
            msg = "   z1:%10.3f z2:%10.3f r1:%10.3f r2:%10.3f z1+tz:%10.3f z2+tz:%10.3f" % ( self.z1co, self.z2co, self.r1co, self.r2co, self.z1co+tz, self.z2co+tz ) 
        elif self.tc == CSG_.BOX3:
            msg = "   x:%10.3f y:%10.3f z:%10.3f  " % ( self.xbox, self.ybox, self.zbox  ) 
        else:
            msg = ""
        pass
        return msg 
        


class Prim(object):
    """

    In [21]: gp[2].primbuf
    Out[21]:
    array([[ 0,  1,  0,  0],
           [ 1,  7,  1,  0],
           [ 8,  7,  5,  0],
           [15,  7,  8,  0],
           [22,  1, 11,  0],
           [23,  7, 12,  0]], dtype=int32)
             |   |   |   | 
     partOffset  |   |   |
                 |   |   |
           numParts  |   |
                     |   |
             tranOffset  |
                         |
                 planOffset
    """
    def __init__(self, primIdx, prim, d):
        """
        """
        assert primIdx > -1 and primIdx < 10000, primIdx
        assert prim.shape == (4,), "unexpected prim.shape %s " % repr(prim.shape)
 
        self.sidx = d.sidx
        self.primIdx = primIdx
        self.prim = prim 

        idx = d.idxbuf[primIdx] if d.idxbuf is not None else -np.ones(4,dtype=np.uint32) 
        lvIdx = idx[2] 

        self.idx = idx
        self.lvIdx = lvIdx

        self.lvName = d.ma.idx2name.get(lvIdx, "-") if d.ma is not None else "--"

        partOffset, numParts, tranOffset, planOffset = prim
        numTran = d.ntran[primIdx]

        parts_ = d.partbuf[partOffset:partOffset+numParts]
        trans_ = d.tranbuf[tranOffset:tranOffset+numTran,0]  # eg shape (2, 4, 4)  plucking the first from the t,v,q triplet of transforms

        self.parts_ = parts_
        self.trans_ = trans_    ## without the python class wrapping

        self.parts = list(map(lambda _:Part(_,trans_,d, self), parts_))   ## note that every part gets passed all the trans_ need to use the gt to determine which one to use

        self.partOffset = partOffset
        self.numParts= numParts
        self.numTran = numTran
        self.tranOffset = tranOffset
        self.planOffset = planOffset
        self.d = d

    def maxdiff(self, other):
        """
        :return float: max difference over the constituent parts, from Part.maxdiff
        """
        assert len(self.parts) == len(other.parts) 
        return max(map( lambda ab:ab[0].maxdiff(ab[1]), zip( self.parts, other.parts)))       

    def tr_maxdiff(self, other):
        """
        :param other: Prim instance to compare self with
        :return value: max absolute difference between the (numtran,4,4) elements 
        """
        return np.max(np.abs(self.trans_ - other.trans_))      

    def __repr__(self):
        fmt = "sidx %2d primIdx %3s idx %30s lvIdx %3d lvName %30s partOffset %3s numParts %3s tranOffset %3s numTran %3s planOffset %3s  " 
        return fmt % (self.sidx, self.primIdx, str(self.idx), self.lvIdx, self.lvName, self.partOffset, self.numParts, self.tranOffset, self.numTran, self.planOffset )  

    def __str__(self):
        return "\n".join(["",repr(self)] + list(map(str,list(filter(lambda pt:pt.tc > 0, self.parts)))) + [repr(self.trans_)]) 


class Solid(object):
    """
    This is Solid in the Opticks composite sense, formerly poorly named Dir

    Handles the concatenated prims from $TMP/GParts/{0,1,2,...} 

    TODO: rename to "Prims" 
    """
    def __init__(self, base, kd):
        """  
        :param base: directory containing primBuffer.npy etc..  eg $TMP/GParts/0


        ::

            In [5]: p.primBuf
            Out[5]: 
            array([[0, 1, 0, 3],
                   [1, 7, 1, 0]], dtype=uint32)

            In [6]: p.partBuf.shape
            Out[6]: (8, 4, 4)

            In [7]: p.partBuf
            Out[7]: 
            array([[[    0.    ,     0.    ,     0.    ,  1000.    ],
                    [    0.    ,     0.    ,     0.    ,     0.    ],
                    [-1000.01  , -1000.01  , -1000.01  ,     0.    ],
                    [ 1000.01  ,  1000.01  ,  1000.01  ,     0.    ]],

                   [[    0.    ,     0.    ,     0.    ,   500.    ],
                    [    0.    ,     0.    ,     0.    ,     0.    ],
                    [ -500.01  ,  -500.01  ,  -500.01  ,     0.    ],
                    [  500.01  ,   500.01  ,   500.01  ,     0.    ]],

        """
        sidx = int(os.path.basename(base))
        self.sidx = sidx
        self.base = base
        self.blib = BLib(kd)  

        primbuf = np.load(os.path.join(base, "primBuffer.npy"))  # "solid" tree level index into part and tran buffers
        idxbuf  = np.load(os.path.join(base, "idxBuffer.npy"))          
        partbuf = np.load(os.path.join(base, "partBuffer.npy"))
        tranbuf = np.load(os.path.join(base, "tranBuffer.npy"))

        assert len(primbuf) == len(idxbuf)
        assert len(tranbuf) <= len(partbuf)

        tot_prim = len(primbuf)
        tot_tran = len(tranbuf)
        tot_part = len(partbuf)   # aka node

        #assert tot_part in [1,3,7,15,31]   nope these are totals for all the prim
        """
                            1                        ->  1    
                 10                  11              ->  3     +2
           100       101        110       111        ->  7     +4
        1000 1001 1010 1011  1100 1101 1110  1111    -> 15     +8 
                                                     -> 31     +16 
        """
        self.primbuf = primbuf
        self.partbuf = partbuf 
        self.tranbuf = tranbuf
        self.idxbuf = idxbuf 

        log.info(str(self))


        ma = Mesh(kd)   # GItemList/GMeshLib.txt LV name2idx idx2name

        lvidxs = idxbuf[:,1]    
        lvns = list(map(lambda lvidx:ma.idx2name[lvidx],lvidxs))
        self.lvns = lvns

        ntran = np.zeros( tot_prim, dtype=np.uint32)
        ntran[0:tot_prim-1] = primbuf[1:,2] - primbuf[:-1,2]    ## differencing the tranOffsets to give numtran

        ntran_tot_excluding_last = ntran.sum()  
        ntran_last = tot_tran - ntran_tot_excluding_last
        ntran[tot_prim-1] = ntran_last           

        self.ntran = ntran 

        self.ma = ma
        self.prims = self.get_prims()
   
    def get_prims(self):
        """
        :return pp: python array of Prim instances deserialized from self.prim array
        """ 
        pp = []

        tot_prim = len(self.primbuf)
        log.debug("get_prims sidx %d tot_prim %d " % (self.sidx, tot_prim))

        for primIdx in range(tot_prim):
            prim_item = self.primbuf[primIdx]
            p = Prim(primIdx, prim_item, self)  
            pp.append(p)
        pass
        return pp

    def enumerate_prim_zip(self, other):
        """
        :return i_ab: (idx,(this_prim,other_prim))  i_ab[0] 
        """ 
        assert len(self.prims) == len(other.prims) 
        return enumerate(zip(self.prims, other.prims))

    def where_discrepant_tr(self, other, cut=0.1):
        assert len(self.prims) == len(other.prims) 
        return map(lambda i_ab:i_ab[0], filter(lambda i_ab:i_ab[1][0].tr_maxdiff(i_ab[1][1]) > cut , enumerate(zip(self.prims, other.prims)) ))

    def where_discrepant_prims(self, other, cut=0.1):
        assert len(self.prims) == len(other.prims) 
        return map(lambda i_ab:i_ab[0], filter(lambda i_ab:i_ab[1][0].maxdiff(i_ab[1][1]) > cut , enumerate(zip(self.prims, other.prims)) ))

    def get_discrepant_prims(self, other, cut=0.1):
        assert len(self.prims) == len(other.prims) 
        return filter(lambda i_ab:i_ab[1][0].maxdiff(i_ab[1][1]) > cut , enumerate(zip(self.prims, other.prims)) )

    def __repr__(self):
        fmt = "Solid %d : %s : primbuf %s partbuf %s tranbuf %s idxbuf %s "  
        return fmt % (self.sidx, self.base, repr(self.primbuf.shape), repr(self.partbuf.shape), repr(self.tranbuf.shape), repr(self.idxbuf.shape))
       

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    ridx = 0 
    ddir = os.path.expandvars(os.path.join("$TMP/GParts",str(ridx)))
    dir_ = sys.argv[1] if len(sys.argv) > 1 else ddir
    sli_ = sys.argv[2] if len(sys.argv) > 2 else "0:10"
    sli = slice(*map(int, sli_.split(":")))

    if dir_ == ddir:
        log.warning("using hardcoded dir" ) ;  
    pass

    from opticks.ana.key import keydir
    kd = keydir(os.environ["OPTICKS_KEY"])

    d = Dir(dir_, kd)
    print("Dir(dir_)", d)

    pp = d.prims
    print("dump sliced prims from the dir slice %s " % repr(sli))
    for p in pp[sli]:
        print(p)
    pass
    #print(d.tran)
  
