#!/usr/bin/env python
"""
ipython -i tests/stranTest.py 

"""
import os, numpy as np
from opticks.ana.fold import Fold

if __name__ == '__main__':
    t = Fold.Load(); 
    print(t)



