#!/usr/bin/env python2

from __future__ import print_function

import sys
sys.path.append('/home/crank/Projects/CERN/na64-meta/install/lib/python2.7/site-packages')

import StromaV.analysis
print( dir(StromaV.analysis) )

ap = StromaV.analysis.AnalysisPipeline()

import code
code.interact(local=locals())
