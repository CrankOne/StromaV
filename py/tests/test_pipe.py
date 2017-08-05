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
import unittest, pickle
from StromaV.pipeline import AnalysisPipeline, iEventProcessor, iEventSequence, dereference_event_ptr_ref
from StromaV.sVEvents import Event

testingDataSeq = [
    #  No, some txt,  do mod, abort#1
      [ 1, "first",   True  , False]
    , [ 2, "second",  True  , False]
    , [ 3, "third",   True  , False]
    , [ 4, "fourth",  False , False]
    , [ 5, "fifth",   True  , True ]
    , [ 6, "sixth",   False , False]
    , [ 7, "seventh", False , True ]
    , [ 8, "eithth",  True  , False]
    , [ 9, "nineth",  True  , False]
]

class MockEventSequence(iEventSequence):
    """
    This class implements a somewhat basic event source class stub that
    generates testing data.
    """
    def pack_data(self, event):
        event.set_blob( pickle.dumps(testingDataSeq[self.idx]) )
        self.idx += 1

    def _V_initialize_reading(self):
        """
        Overloaded function called once the reading process begins.
        """
        self.reentrantEvent = Event()
        self.idx = 0
        self.pack_data( self.reentrantEvent )
        return self.reentrantEvent

    def _V_is_good(self):
        return self.idx < len(testingDataSeq)

    def _V_next_event(self, eventRef):
        evPtr = dereference_event_ptr_ref(eventRef)
        # NOTE, strange:
        #print( str(evPtr), str(self.reentrantEvent) )
        #self.ut.assertEqual( int(evPtr), int(self.reentrantEvent) )
        self.pack_data( evPtr )

    def _V_finalize_reading(self):
        pass

    def __init__(self, ut):
        self.ut = ut
        self.reentrantEvent = None
        super(MockEventSequence, self).__init__( 0 )

class MockEventProcessor1(iEventProcessor):
    """
    This class implements a basic event processor class stub that
    receives a testing events data f]om MockEventSequence instance and
    tries to validate it. It also selectively discriminates some events to
    check basic pipeline logic.]
    """
    def __init__(self, ut):
        self.ut = ut
        self.idx = 1  # will be compared with index passing through the processor
        super(MockEventProcessor1, self).__init__('mock-1')

    def _V_process_event(self, event):
        ret = iEventProcessor.RC_ACCOUNTED
        # This way one may check to which type (within the "oneof" protobuf
        # struct) the event is belonging:
        self.ut.assertEqual( event.uevent_case(), Event.kBlob )
        # ^^^ i.e., if this event will bring the simulated/experimental payload
        # this as]ertion will fall.
        entry = pickle.loads( event.blob() )
        self.ut.assertEqual( self.idx, entry[0] )
        if entry[2]:
            entry.append({'extra-data' : self.idx})
            event.set_blob( pickle.dumps(entry) )
            ret = iEventProcessor.RC_CORRECTED
        if entry[3]:
            ret |= iEventProcessor.ABORT_CURRENT
        if entry[0] == 8:
            ret &= ~iEventProcessor.CONTINUE_PROCESSING
        self.idx += 1
        return ret

class MockEventProcessor2(iEventProcessor):
    """
    This class implements a basic event processor class stub that
    receives a testing events data from MockEventProcessor2, with all the
    transformations applied by MockEventProcessor1.
    """
    def __init__(self, ut):
        self.ut = ut
        super(MockEventProcessor2, self).__init__('mock-2')

    def _V_process_event(self, event):
        entry = pickle.loads( event.blob() )
        print( entry )
        if entry[2]:
            self.ut.assertEqual( entry[0], entry[4]['extra-data'] )
        self.ut.assertFalse( entry[3] )
        self.ut.assertTrue( entry[0] < 8 )
        return iEventProcessor.RC_ACCOUNTED

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
        self.pipeline.process( self.source )

if __name__ == "__main__":
    unittest.main()
