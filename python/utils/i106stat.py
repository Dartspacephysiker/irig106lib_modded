'''
Created on Dec 21, 2011

@author: rb45
'''

import sys
import Py106
#from irig106 import Packet
#import irig106.Packet

# ---------------------------------------------------------------------------

# This test code just opens an IRIG file and does a histogram of the 
# data types

#IrigStatus = PacketIO.Status()
IrigIO     = Py106.Packet.IO()

# Initialize counts variables
Counts = {}
#DataType = Py106.Packet.DataType()

if len(sys.argv) > 1 :
    Status = IrigIO.open(sys.argv[1], Py106.Packet.FileMode.READ)
    if Status != Py106.Packet.Status.OK :
        print "Error opening data file %s" % (sys.argv[1])
        sys.exit(1)
else :
    print "Usage : irig106.py <filename>"
    sys.exit(1)

while True:
    Status = IrigIO.read_next_header()
    if Status != Py106.Packet.Status.OK:
        break

    if Counts.has_key(IrigIO.Header.DataType):
        Counts[IrigIO.Header.DataType] += 1
    else:
        Counts[IrigIO.Header.DataType]  = 1

IrigIO.close()

for DataTypeNum in Counts:
    print "Data Type %-16s Counts = %d" % ( Py106.Packet.DataType.TypeName(DataTypeNum),  Counts[DataTypeNum])
