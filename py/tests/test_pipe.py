# -*- coding: utf-8 -*-
# Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
# Author: Renat R. Dusaev <crank@qcrypt.org>
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
This testing unit checks the pipeline classes basics: the AnalysisPipeline,
iEventSequence, iEventProcessor and the event wrappers.
"""

from __future__ import print_function
import unittest
from StromaV.pipeline import AnalysisPipeline, iEventProcessor, iEventSequence

class MockEventSequence(iEventSequence):
    """
    This class implements a somewhat basic event source class stub that
    generates testing data.
    """
    def __init__(self, ut):
        self.ut = ut
        super(MockEventSequence, self).__init__(self)

class MockEventProcessor1(iEventProcessor):
    """
    This class implements a basic event processor class stub that
    receives a testing events data from MockEventSequence instance and
    tries to validate it. It also selectively discriminates some events to
    check basic pipeline logic.
    """
    def __init__(self, ut):
        self.ut = ut
        super(MockEventProcessor1, self).__init__(self)

class MockEventProcessor2(iEventProcessor):
    """
    This class implements a basic event processor class stub that
    receives a testing events data from MockEventProcessor2, with all the
    transformations applied by MockEventProcessor1.
    """
    def __init__(self):
        self.ut = ut
        super(MockEventProcessor2, self).__init__(self)

#
# Test Cases
###########

class TestMockingPipeline(unittest.TestCase):
    """
    This case performs data transmission within StromaV pipeline using mock
    classes.
    """
    def setUp(self):
        """
        Creates a pipeline consisting with data source and mock processor.
        """
        # instantiate couple processors
        self.proc1 = MockEventProcessor1(self)
        self.proc2 = MockEventProcessor2(self)
        # instantiate a pipeline
        self.pipeline = AnalysisPipeline()
        # stack these few processors in a pipeline
        self.pipeline.push_back_processor( self.proc1 )
        self.pipeline.push_back_processor( self.proc2 )
        # instantiate a data source for further use
        self.source = MockEventSequence(self)

    def test_dataflow_logic(self):
        # generate few events and transmit it via the pipeline
        self.pipeline( self.source )
