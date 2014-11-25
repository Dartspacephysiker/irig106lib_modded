# Example 1

import sys

# Import the irig106 package
import Py106
import Py106.MsgDecode1553

# Create IRIG IO object
IrigIO  = Py106.Packet.IO()

# Open data file for reading
if len(sys.argv) > 1 :
    RetStatus = IrigIO.open(sys.argv[1], Py106.Packet.FileMode.READ)
    if RetStatus != Py106.Status.OK :
        print "Error opening data file %s" % (sys.argv[1])
        sys.exit(1)
else :
    print "Usage : example_1.py <filename>"
    sys.exit(1)

# Read IRIG headers
for PktHdr in IrigIO.packet_headers():
    print "Ch ID %3i  %s" % (IrigIO.Header.ChID, Py106.Packet.DataType.TypeName(PktHdr.DataType))
            
IrigIO.close()
