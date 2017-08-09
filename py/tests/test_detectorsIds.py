# -*- coding: utf-8 -*-
# Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
# Author: Renat R. Dusaev <crank@qcrypt.org>
# Author: Bogdan Vasilishin <togetherwithra@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""
Python testing unit performs checks of the na64_detectors_ids's functions
09.08.17 -- simple testing
"""

from __future__ import print_function
import unittest

# TODO: move to afNA64 and delete two lines velow:
# import StromaV.appUtils
# app = StromaV.appUtils.PythonSession.init_from_string( 'pyApp', 'Some python application', '' )

import afNA64.na64DetectorIds as dids

class TestDetTables(unittest.TestCase):
    """
    Check certain family name concurrence with obtained (through a couple
    of functions) one.
    Testing detector_major_by_name().
    """
    def test_mjn_transmissions(self):
        mjNo = dids.detector_major_by_name( 'ECAL0' )
        detFamId = dids.detector_family_num( mjNo )
        detFamName = dids.detector_family_name( detFamId )
        # Test family name
        self.assertEqual( detFamName, 'ECAL')
        # Test results from detector_major_by_name()
        self.assertEqual( mjNo, dids.EnumScope.d_ECAL0)



