from StromaV.gooDict import Dictionary

dct = Dictionary( "tst", "Testing dictionary" )

dct.insertion_proxy()  \
    .p( int, shortcut='a', name="int-parameter",
        description="Parameter 1, int." )  \
    .p( float, shortcut='b', name="float-parameter",
        description="Parameter 2, float." )  \
    .p( bool, shortcut='c',
        description="Parameter 3, bool.",
        default=True )  \
    .p( str, name='char-parameter',
        description="Parameter 4, str.",
        default='this is a string' )  \
    .p( (int,), shortcut='e', name='tuple-parameter',
        description="Parameter 5, str.", )

# See: https://stackoverflow.com/questions/35282222/in-python-how-do-i-cast-a-class-object-to-a-dict
#def __iter__(self):
#    yield 'a', self.a
#    yield 'b', self.b
#    yield 'c', self.c


