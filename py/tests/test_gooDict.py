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

from __future__ import print_function
import unittest
from castlib3.models.filesystem import Folder, File, DeclBase
from StromaV.gooDict import Dictionary

stringToBeChecked='Klaata baradu nikto.'

class TestDictionaryBasics(unittest.TestCase):
    def setUp(self):
        self.dct = Dictionary( "test", "Testing dictionary." )
        self.dct.insertion_proxy()  \
            .p( int, shortcut='a', name="int-parameter",
                description="Parameter 1, int.", default=12 )  \
            .p( float, shortcut='b', name="float-parameter",
                description="Parameter 2, float.")  \
            .p( bool, shortcut='c',
                description="Parameter 3, bool.",
                default=True )  \
            .p( str, name='string-parameter',
                description="Parameter 4, str.",
                default=stringToBeChecked )  \
            .p( (int,), shortcut='e', name='tuple-parameter',
                description="Parameter 5, str.", )

    def test_base_access(self):
        # This will test dictionary attribute set to default vale:
        self.assertTrue( hasattr( self.dct, 'int-parameter' ) )
        self.assertTrue( self.dct.int_parameter == 12 )
        self.assertTrue( self.dct.string_parameter == stringToBeChecked )

    def test_no_key(self):
        self.assertRaises( KeyError, getattr, self.dct, 'blam' )

    #def test_inconsistent(self):
    # Has no sense without recursive traversal
    #    self.assertFalse( self.dct.inconsistent )

class TestDictionaryAdvanced( TestDictionaryBasics ):
    def setUp(self):
        super( TestDictionaryAdvanced, self ).setUp()
        ip = self.dct.insertion_proxy()
        ip.bgn_sect('sub1', "Testing subsection #1")  \
                .p( int, name='int-sub1', description='Some int-typed parameter.' ) \
                .p( int, name='int-sub2', description='Another int-typed parameter.' ) \
                .bgn_sect('subsub1', "Testing subsection #2") \
                    .p( float, name='int-sub1', description='Some float-typed parameter.' ) \
                .end_sect('subsub1') \
            .end_sect('sub1')

    #def test_
    

# See: https://stackoverflow.com/questions/35282222/in-python-how-do-i-cast-a-class-object-to-a-dict
#def __iter__(self):
#    yield 'a', self.a
#    yield 'b', self.b
#    yield 'c', self.c


