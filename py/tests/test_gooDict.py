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
This testing unit performs check of the goo::dict::Dictionary class bindings.
It is also a good check for the attendant InsertionProxy class.
"""

from __future__ import print_function
import unittest
from StromaV.gooDict import Dictionary
from StromaV import GooException

stringToBeChecked='Klaata baradu nikto.'
floatValueToBeChecked=1.476e+2
intValueToBeChecked=42

class TestDictionaryBasics(unittest.TestCase):
    """
    Tests most basic dictionary functions.
    """
    def setUp(self):
        """
        Creates a basic dictionary of float structure to test basic
        """
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
                description="Parameter 5, str." ) \
            .p( int, name='int-parameter-to-set',
                description='This parameter has to be set and its value '
                'will be checked further.')

    def test_base_access(self):
        """
        Check dictionary initial state.
        """
        # This may be extended further, so we need to check that AT LEAST the
        # inserted parameters are present.
        self.assertTrue(
            set(self.dct.parameters()).issuperset( (
                'int-parameter', 'float-parameter', 'c',
                'string-parameter', 'tuple-parameter' ) ) )
        # This will test dictionary attribute set to default vale:
        self.assertTrue( hasattr( self.dct, 'int-parameter' ) )
        self.assertTrue( self.dct.int_parameter == 12 )
        self.assertTrue( self.dct.string_parameter == stringToBeChecked )
        self.assertTrue( self.dct.c )
        self.assertTrue(
            type( self.dct.tuple_parameter ) is tuple and
            0 == len(self.dct.tuple_parameter) )

    def test_no_key(self):
        """
        Checks acquizition by non-existing key and unset parameter. The
        KeyError and GooException have to be raised correspondingly.
        """
        self.assertRaises( KeyError, getattr, self.dct, 'blam' )
        # Shall raise "has not been set while its value required" exception.
        # Since GooException is not slightly typed, we just testing its its
        # message here
        self.assertRaisesRegexp( GooException,
                "has not been set while its value required",
                getattr, self.dct, 'float-parameter' )



class TestDictionaryAdvanced( TestDictionaryBasics ):
    """
    Tests some advanced functions within the dictionary facility: folded
    structure, assignment of scalar and tuples, type casting, etc.
    """
    def setUp(self):
        super( TestDictionaryAdvanced, self ).setUp()
        ip = self.dct.insertion_proxy()
        ip.bgn_sect('sub1', "Testing subsection #1")  \
                .p( int, name='int-sub1', description='Some int-typed parameter.' ) \
                .p( int, name='int-sub2', description='Another int-typed parameter.' ) \
                .bgn_sect('subsub1', "Testing subsection #2") \
                    .p( float,  name='float-sub1',  description='Some int-typed parameter.' ) \
                    .p( int,    name='int-sub1',    description='Another float-typed parameter.' ) \
                    .p( bool,   name='bool-sub1',   description='Yet another bool-typed parameter.' ) \
                    .p( (float,), shortcut='F', description='Some list of floats to ' \
                        'check tuple acquizition', default=(1.276, 512, 2.1e-32) ) \
                    .p( float, name='flt_param_test', description='Some float to extract',
                        default=floatValueToBeChecked ) \
                    .p( (int,), shortcut='I', description='Tuple of integers to be set.' ) \
                    .p( (bool,), shortcut='B', description='Tuple of bools to be set.' ) \
                .end_sect('subsub1') \
            .end_sect('sub1') \
            .bgn_sect('sub2', 'Empty subsection.').end_sect('sub2') \
            .p( str, name='string-parameter-2',
                    description='Some string parameter to check.' ) \
            .p( bool, name='bool-parameter' )
        ip.p( (int,), name='int-list', description='Some list of integers to ' \
                'check tuple acquizition', default=(1, 2, 3, 4, 5) ) \
            .p( (str,), shortcut='s', description='Some list of strings to ' \
                'check tuple acquizition', default=('foo', 'bar') ) \
            .p( (int,), name='int-list-2', description='Some list of integers to ' \
                'check tuple assignment', default=(1, 2, 3) ) \
            .p( (str,), name='str-list-2', description='Some list of strings to ' \
                'check tuple assignment.', default=('me unused',) ) \
        # Check insertion of parameter with non-uniq name:
        self.assertRaisesRegexp( GooException,
                "Duplicated option name",
                ip.p, int, name='int-parameter-to-set' )

    def test_list_access(self):
        self.assertTrue( self.dct.int_list, (1, 2, 3, 4, 5) )
        self.assertTrue( self.dct.s,        ("foo", "bar") )

    def test_subsect_access(self):
        self.assertTrue(set(self.dct.dictionaries()).issuperset(( 'sub1', 'sub2' ) ))
        self.assertTrue(type(self.dct.subsection('sub1')) is Dictionary)
        self.assertTrue(type(self.dct.sub1) is Dictionary)
        self.assertTrue(type(self.dct.sub2) is Dictionary)
        self.assertTrue(type(self.dct.sub1.subsub1) is Dictionary)
        self.assertAlmostEqual(self.dct.sub1.subsub1.flt_param_test, floatValueToBeChecked)

    def test_scalar_parameter_setting( self ):
        self.assertRaisesRegexp( GooException,
                "has not been set while its value required",
                getattr, self.dct, 'int-parameter-to-set' )
        # setting integer parameter
        self.dct.int_parameter_to_set = intValueToBeChecked
        self.assertEqual( self.dct.int_parameter_to_set, intValueToBeChecked )
        self.dct.int_parameter_to_set = 0
        self.assertEqual( self.dct.int_parameter_to_set, 0 )
        # To check the from-string parsing capabilities:
        self.dct.set_from_str( 'int-parameter-to-set', hex(-intValueToBeChecked) )
        self.assertEqual( self.dct.int_parameter_to_set, -intValueToBeChecked )
        # The = 'whatever' stlye is prohibited (considered as a bad style)
        self.assertRaisesRegexp( GooException,
                "can not be implicitly",
                setattr, self.dct, 'int-parameter-to-set', 'blam!' )
        # setting float parameter
        self.dct.sub1.subsub1.flt_param_test = 2*floatValueToBeChecked
        self.assertAlmostEqual(self.dct.sub1.subsub1.flt_param_test, 2*floatValueToBeChecked)
        self.dct.sub1.subsub1.flt_param_test = 3*floatValueToBeChecked
        self.assertAlmostEqual(self.dct.sub1.subsub1.flt_param_test, 3*floatValueToBeChecked)
        # setting bool parameter
        #   NOTE: bool parameters are always set to false by default. Attempt
        #   to get them DOES NOT raises the exception when they're not set.
        #self.dct.bool_parameter = False
        self.assertFalse( self.dct.bool_parameter )
        self.dct.bool_parameter = True
        self.assertTrue( self.dct.bool_parameter )
        # Check parsing
        self.dct.set_from_str( 'bool-parameter', 'no' )
        self.assertFalse( self.dct.bool_parameter )
        # setting string parameter
        self.dct.string_parameter_2 = stringToBeChecked
        self.assertEqual( self.dct.string_parameter_2, stringToBeChecked )

    def test_scalar_parameter_casting_Setting(self):
        # setting integer parameter from floating point
        self.dct.int_parameter_to_set = floatValueToBeChecked
        self.assertEqual( self.dct.int_parameter_to_set, int(floatValueToBeChecked) )
        self.dct.int_parameter_to_set = 0.
        self.assertEqual( self.dct.int_parameter_to_set, 0. )
        # setting float parameter from integer
        self.dct.sub1.subsub1.flt_param_test = 2*intValueToBeChecked
        self.assertAlmostEqual(self.dct.sub1.subsub1.flt_param_test, 2*intValueToBeChecked)
        self.dct.sub1.subsub1.flt_param_test = 3*intValueToBeChecked
        self.assertAlmostEqual(self.dct.sub1.subsub1.flt_param_test, 3*intValueToBeChecked)
        # setting bool parameter from floating point and integer
        self.dct.bool_parameter = False
        self.assertFalse( self.dct.bool_parameter )
        self.dct.bool_parameter = intValueToBeChecked
        self.assertTrue( self.dct.bool_parameter )
        self.dct.bool_parameter = False
        self.assertFalse( self.dct.bool_parameter )
        self.dct.bool_parameter = floatValueToBeChecked
        self.assertTrue( self.dct.bool_parameter )
        # setting string parameter
        self.dct.string_parameter_2 = stringToBeChecked
        self.assertEqual( self.dct.string_parameter_2, stringToBeChecked )
        # TODO: this is a question to answer: shall we provide the parsing
        # mechanism for setting a parameters of complex types within usual
        # assignment operator? This involves parsing of a parameter from
        # string, e.g.:
        #self.dct.path_parameter = '$(ENV:LD_LIBRARY_PATH)/libSome.so'
        #self.dct.histogram = '100[-1:1]'
        # Wouldn't it be better to implement a dedicated mechanism, like:
        #self.dct.set_from_string('path_parameter', '/some/path')

    def test_tuple_parameter_setting(self):
        self.dct.int_list_2 = (3, 2, 1)
        self.assertEqual( self.dct.int_list_2, (3, 2, 1) )
        self.dct.str_list_2 = ('me', 'used', 'to check', 'assignment')
        self.assertEqual( self.dct.str_list_2, ('me', 'used', 'to check', 'assignment') )

    def test_implicit_type_conversion(self):
        # Check scalar parameter setting
        dct = self.dct.sub1.subsub1
        # Note, that dct hereby is a reference-proxying object (not a copy!)
        dct.float_sub1 = intValueToBeChecked
        dct.int_sub1 = floatValueToBeChecked
        dct.bool_sub1 = 115
        self.assertEqual( self.dct.sub1.subsub1.float_sub1, float(intValueToBeChecked) )
        self.assertEqual( self.dct.sub1.subsub1.int_sub1, int(floatValueToBeChecked) )
        self.assertTrue( self.dct.sub1.subsub1.bool_sub1 )
        # Check the list type casting:
        self.assertEqual( self.dct.sub1.subsub1.F, (1.276, 512, 2.1e-32) )
        dct.F = (115, 225, 335)
        self.assertAlmostEqual( self.dct.sub1.subsub1.F, (float(115), float(225), float(335)) )
        dct.I = (floatValueToBeChecked*2, floatValueToBeChecked, 1.5*floatValueToBeChecked)
        self.assertAlmostEqual( self.dct.sub1.subsub1.I,
                ( int(floatValueToBeChecked*2),
                  int(floatValueToBeChecked),
                  int(1.5*floatValueToBeChecked) ) )
        dct.B = ( 'some', '', None, [1,], [], 1.23, {} )
        self.assertEqual( self.dct.sub1.subsub1.B,
                ( True,   # non-empty string
                  False,  # empty string
                  False,  # None
                  True,   # non-empty list
                  False,  # empty list
                  True,   # non-zero float value
                  False,  # empty dict
                  ) )

    #def test_inconsistent(self):
    # Has no sense without recursive traversal
    #    self.assertFalse( self.dct.inconsistent )


#
# Integration of unwrapped parameter type declared in C++ into Python
from StromaV.extParameters import \
            PType_Path, Path, \
            PType_HistogramParameters1D, HistogramParameters1D, \
            PType_HistogramParameters2D, HistogramParameters2D, \
            PType_G4ThreeVector, G4ThreeVector, \
            iSingularParameter

class TestDictionaryCustomTypes(unittest.TestCase):
    """
    Tests foreign parameters integration.
    """
    def setUp(self):
        self.dct = Dictionary( "test", "Testing dictionary." )
        self.dct.insertion_proxy()  \
            .p( PType_Path, 'some-path', 'Path parameter.' )
        self.assertTrue( issubclass(PType_Path, iSingularParameter) )

    def test_foreign_parameter(self):
        # Set from wrapped object (available only for wrapped parameter classes):
        self.dct.some_path = Path( '/bin/bash' )
        self.assertEqual( '/bin/bash', self.dct.strval_of( 'some-path' ) )
        # Alternative way (available for any parameter class):
        self.dct.set_from_str( 'some-path', '/bin/sh' )
        self.assertEqual( '/bin/sh', self.dct.strval_of( 'some-path' ) )
        # Retrieve as a foreign type:
        p = Path(self.dct['some-path'])
        self.assertEqual( '/bin/sh', p.interpolated() )

class TestHistogramParametersWrap(unittest.TestCase):
    """
    Tests foreign parameters integration: HistogramParameters[1D,2D]
    """
    def setUp(self):
        self.assertTrue( issubclass(PType_HistogramParameters1D, iSingularParameter) )
        self.assertTrue( issubclass(PType_HistogramParameters2D, iSingularParameter) )
        self.dct = Dictionary( "test", "Testing dictionary." )
        self.dct.insertion_proxy()  \
            .p( PType_HistogramParameters1D, 'hst1D', 'Histogram 1D parameter.',
                    HistogramParameters1D(11, -15, 16.5) ) \
            .p( PType_HistogramParameters2D, 'hst2D', 'Histogram 2D parameter.',
                    HistogramParameters2D(11, -14, 14, 21, -43, 55) )

    def test_hst(self):
        self.assertEqual( self.dct.strval_of( 'hst1D' ), '{11[-15:16.5]}' )
        self.dct.hst1D = HistogramParameters1D( 100, -1, 1 )
        self.assertEqual( self.dct.strval_of( 'hst1D' ), '{100[-1:1]}' )
        self.assertEqual( self.dct.strval_of( 'hst2D' ), '{11[-14:14]x21[-43:55]}' )
        self.dct.hst2D = HistogramParameters2D( 100, -1, 1, 200, -2, 2 )
        self.assertEqual( self.dct.strval_of( 'hst2D' ), '{100[-1:1]x200[-2:2]}' )

class TestG4ThreeVectorWrap(unittest.TestCase):
    """
    Tests foreign parameters integration: Hep3Vector (G4ThreeVector).
    """
    def setUp(self):
        self.assertTrue( issubclass(PType_G4ThreeVector, iSingularParameter) )
        self.dct = Dictionary( "test", "Testing dictionary." )
        self.dct.insertion_proxy()  \
            .p( PType_G4ThreeVector, 'somevec', 'Some 3D vector to test.',
                    G4ThreeVector(1., 2.5, 3.) )

    def test_G4ThreeVector(self):
        v = G4ThreeVector( self.dct['somevec'] )
        self.assertAlmostEqual( (1., 2.5, 3.),
            (v.x(), v.y(), v.z()) )

#
##
## Integration of parameter type declared in python into C++
#from StromaV.gooDict import PyParameter
#
#class SomeCustomType(object):
#    __metaclass__ = PyParameter
#
#    def getter_one(self):
#        # ... may performs some operations here
#        return intValueToBeChecked
#
#    def getter_two(self):
#        # ... may performs some operations here
#        return floatValueToBeChecked
#
#    # This dictionary may be dynamically utilized from within C++.
#    __getters = {
#            'one' : (int,   setter_one, getter_one),
#            'two' : (float, setter_two, getter_one)
#        }
#
#
#class TestDictionaryCustomTypes(unittest.TestCase):
#    """
#    Tests custom types integration.
#    """
#    def setUp(self):
#        self.dct = Dictionary( "test", "Testing dictionary." )
#        self.dct.insertion_proxy()  \
#            .p( SomeCustomType, name='some-typed',
#                    description='Parameter of custom type.' )
#
#    def test_foreign_parameter(self):
#        self.assertTrue(type(self.dct.some_typed) is SomeCustomType)
#        # TODO: operate with bindings here


