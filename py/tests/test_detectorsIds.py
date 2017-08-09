from __future__ import print_function
import StromaV.appUtils
app = StromaV.appUtils.PythonSession.init_from_string( 'pyApp', 'Some python application', '' )

import afNA64.na64DetectorIds as dids

mjNo1 = dids.detector_major_by_name('ECAL0')
mjNo2 = dids.detector_major_by_name('ECAL1')
mjNo3 = dids.detector_major_by_name('HCAL0')

print(mjNo1)
print(mjNo2)
print(mjNo3)
