#!/usr/bin/env python
"""

"""

import numpy as np
import os, logging, json, pprint
pp = pprint.PrettyPrinter(indent=4)

expand_ = lambda path:os.path.expandvars(os.path.expanduser(path))
json_load_ = lambda path:json.load(file(expand_(path)))
json_save_ = lambda path, d:json.dump(d, file(expand_(path),"w"))



log = logging.getLogger(__name__)

from opticks.ana.base import opticks_main
from opticks.ana.nbase import find_ranges
from opticks.ana.pmt.treebase import Tree
from opticks.ana.pmt.gdml import GDML
from opticks.ana.pmt.polyconfig import PolyConfig
from opticks.dev.csg.textgrid import TextGrid

from opticks.dev.csg.csg import CSG  


class SNode(object):
    """
    Solid node for establishing relationships between all solids within 
    a geometry, especially looking for top solids that are not
    contained within any other solid.
    """
    registry = {}

    @classmethod
    def create(cls, idx, ssidx=[]):
        sn = SNode(idx, ssidx)
        cls.registry[idx] = sn 
        log.debug("SNode.create %r " % (sn))
        for ssi in ssidx:
            if ssi == idx:continue
            child = cls.registry[ssi]
            assert child
            child.top = sn  
            # top for some SNode will change as more SNode are created, 
            # but dont care, are just looking for SNode without tops
        pass
        return sn

    @classmethod
    def tops(cls, ssmin=0):
        return filter(lambda sn:sn.top is None and sn.ssn >= ssmin, cls.registry.values())

    def __repr__(self):
        return "%3d; %s " % (self.idx, self.ssd)

    def __init__(self, idx, ssidx ):
        self.idx = idx
        self.ss = ssidx
        ssr = list(find_ranges(sorted(ssidx)))
        assert len(ssr) in (0,1)  
        if len(ssr) == 1:
            ssr = ssr[0]
            ssn = ssr[1] - ssr[0] + 1 
            ssd = "%2d:%3d-%3d" % (ssn, ssr[0], ssr[1])
        elif len(ssr) == 0:
            ssr = None
            ssn = 0 
            ssd = "-"
        pass
        self.ssr = ssr 
        self.ssn = ssn
        self.ssd = ssd 
        self.top = None
        


class Scene(object):

    base = "$TMP/dev/csg/scene" 
    name = "scene.json" 

    @classmethod
    def path_(cls):
        base = expand_(cls.base) 
        return os.path.join(cls.base, cls.name)

    path = property(lambda self:self.path_())

    def __init__(self, gdml):
        self.gdml = gdml
        self.associate_solids_to_lv()

    def _get_gltf(self):
        g = {}
        g["scenes"] = [{"nodes":[0]}]
        g["nodes"] = [{"mesh":0 }]
        g['asset'] = dict(version="2.0", generator="scene.py", copyright="Opticks")
        return g
    gltf = property(_get_gltf)

    def save(self):
        self.save_lvsolids()
        #self.save_materials()
        self.save_gltf() 


    def associate_solids_to_lv(self):
        so2lv = {}
        lvs = self.gdml.volumes.values()
        for lv in lvs:
            solid = lv.solid
            so2lv[solid.idx] = lv.idx
            pass
        pass
        self.so2lv = so2lv

    def analyse_solids(self):
        """
        Builds tree of SNode in order to identify 
        top solids that are not part of any other solid.
        """
        flatsolids = self.gdml.solids.values()
        for solid in flatsolids:
            sn = SNode.create( solid.idx, solid.subsolids )
            ssr = solid.subsolidranges
            assert len(ssr) in (0,1), "all solids expected to be leaves with no subsolids or have contiguous idx range subsolids"
        pass
        tops = SNode.tops()
        ntops = len(tops)
        ndeep = 0 
        deeplv = []
   
        for top in tops:
            solid = self.gdml.solids(top.idx) 
            cn = solid.as_ncsg()
            cn.analyse() 

            lvidx = self.so2lv[solid.idx]
            lv = self.gdml.volumes(lvidx)

            if cn.height > 3: 
                ndeep += 1 
                print "solid.idx:%d cn.height:%d cn.totnodes:%d solid.name:%s ideep:%d lvidx:%d lvn:%s " % (solid.idx, cn.height, cn.totnodes, solid.name, ndeep, lvidx, lv.name  )
                deeplv.extend(["%2d : %-60s : %s " % (ndeep, lv.name,repr(cn))])
                print cn.txt
            pass
        pass
        log.info("analyse_solids nflatsolids:%d ntops:%d ndeep:%d " % (len(flatsolids), ntops, ndeep)) 
        
        print "\n".join(deeplv)


    def save_lvsolids(self):
        """
        TODO: Needs to save only solids referenced from a subtree, but use
        absolute indices from the full gdml
        """ 
        rdir = self.prep_reldir("lvsolids")
        lvs = self.gdml.volumes.values()
        for lv in lvs:
            solid = lv.solid
            ssn = solid.subsolidcount 
            cn = solid.as_ncsg()
            cn.analyse() 
            treedir = os.path.join(rdir, "%d" % lv.idx )
            cn.save(treedir)
            pass
        pass
        log.info("save_lvsolids nlvs:%d " % (len(lvs))) 

    def prep_reldir(self, reldir):
        rdir = os.path.join(self.base, reldir)
        if not os.path.exists(rdir):
            os.makedirs(rdir)
        pass
        return rdir

    def save_gltf(self):
        path = self.path 
        log.info("save_gltf to %s " % path )
        json_save_(path, dict(self.gltf))

    @classmethod 
    def load_gltf(cls, path=None):
        if path is None:
            path = cls.path_()  
        pass
        gltf = json_load_(path)
        pp.pprint(gltf)
        return gltf 



class Nd(object):
    """
    Mimimal representation of a node tree, just holding 
    referencing indices and transforms
    """
    count = 0 
    ulv = set()
    uso = set()
    registry = {}

    def __init__(self, nindex, lvidx, transform, name, depth):
        self.nindex= nindex
        self.lvidx = lvidx 
        self.name = name
        self.transform = transform
        self.depth = depth
        self.ichildren = []

    def _get_gltf(self):
        d = {}
        d["name"] = self.name
        d["children"] = self.ichildren
        d["matrix"] = self.smatrix
        return d
    gltf = property(_get_gltf)

    stransform = property(lambda self:"".join(map(lambda row:"%30s" % row, self.transform )))

    def _get_smatrix(self):
        return ",\n".join(map(lambda row:",".join(map(lambda v:"%10s" % v,row)), self.transform )) 
    smatrix = property(_get_smatrix)

    brief = property(lambda self:"Nd ni%5d  lv%5d nch:%d chulv:%s  st:%s " % (self.nindex, self.lvidx,  len(self.ichildren), self.ch_unique_lv, self.stransform ))

    ch_unique_lv = property(lambda self:list(set(map(lambda ch:ch.lvidx, self.children))))

    children = property(lambda self:map(lambda ic:self.get(ic), self.ichildren))

    def __repr__(self):
        indent = ".. " * self.depth 
        return "\n".join([indent + self.brief] + map(repr, self.children))   

    @classmethod
    def report(cls):
        log.info(" count %d len(ulv):%d len(uso):%d " % (cls.count, len(cls.ulv), len(cls.uso)))

    @classmethod
    def summarize(cls, node, depth):
        cls.count += 1
        cls.ulv.add(node.lv.idx)
        cls.uso.add(node.lv.solid.idx)

        name = node.lv.name  # <-- hmm whats the best name ? 
        nindex = node.index

        nd = cls(nindex, node.lv.idx, node.pv.transform, name, depth)
        assert not nindex in cls.registry
        cls.registry[nindex] = nd  
        return nd  

    @classmethod
    def get(cls, nindex):
        return cls.registry[nindex]

    @classmethod
    def build_minimal_tree(cls, target):
        def build_r(node, depth=0):
            if depth < 3:
                nd = cls.summarize(node, depth)
            else:
                nd = None 
            pass
            for child in node.children: 
                ch = build_r(child, depth+1)
                if nd is not None and ch is not None:
                    nd.ichildren.append(ch.nindex)
                pass
            pass
            return nd
        pass 
        return build_r(target)





if __name__ == '__main__':

    args = opticks_main()

    gsel = args.gsel            # string representing target node index integer or lvname
    gmaxnode = args.gmaxnode    # limit subtree node count
    gmaxdepth = args.gmaxdepth  # limit subtree node depth from the target node
    gidx = args.gidx            # target selection index, used when the gsel-ection yields multiple nodes eg when using lvname selection 

    gsel = "/dd/Geometry/AD/lvSST0x" 
    gmaxdepth = 3


    log.info(" gsel:%s gidx:%s gmaxnode:%s gmaxdepth:%s " % (gsel, gidx, gmaxnode, gmaxdepth))


    gdmlpath = os.environ['OPTICKS_GDMLPATH']   # set within opticks_main 
    gdml = GDML.parse(gdmlpath)


    tree = Tree(gdml.world)
    target = tree.findnode(gsel, gidx)
    log.info(" target node %s " % target )   
    nodelist = target.rprogeny(gmaxdepth, gmaxnode)
    log.info(" target nodelist  %s " % len(nodelist) )   


    nd = Nd.build_minimal_tree(target)
    Nd.report() 



if 0:

    scene = Scene(gdml)
    #scene.analyse_solids()
    scene.save_lvsolids()
    gltf = Scene.load_gltf()







