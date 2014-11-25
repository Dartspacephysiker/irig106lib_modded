# Example 2

import sys

# Import the irig106 package
import Py106
import Py106.Time
import Py106.MsgDecode1553

# Create IRIG IO object
IrigIO     = Py106.Packet.IO()
IrigTime   = Py106.Time.Time(IrigIO)
Decode1553 = Py106.MsgDecode1553.Decode1553F1(IrigIO)

TR = ("R", "T")


# Open data file for reading
if len(sys.argv) > 1 :
    RetStatus = IrigIO.open(sys.argv[1], Py106.Packet.FileMode.READ)
    if RetStatus != Py106.Status.OK :
        print "Error opening data file %s" % (sys.argv[1])
        sys.exit(1)
else :
    print "Usage : example_2.py <filename>"
    sys.exit(1)

# Get time sync'ed up
IrigTime.SyncTime(False, 10)

# Read IRIG headers
for PktHdr in IrigIO.packet_headers():
    if PktHdr.DataType == Py106.Packet.DataType.MIL1553_FMT_1:
        IrigIO.read_data()
        for Msg in Decode1553.msgs():
            WC = Decode1553.word_cnt(Msg.pCmdWord1.contents.Value)
            sys.stdout.write ("%16x Ch %3i   %2i-%s-%02i-%02i (%04X)  " % (  \
                Msg.p1553Hdr.contents.PktTime,          \
                IrigIO.Header.ChID,                     \
                Msg.pCmdWord1.contents.Field.RTAddr,    \
                TR[Msg.pCmdWord1.contents.Field.TR],    \
                Msg.pCmdWord1.contents.Field.SubAddr,   \
                Msg.pCmdWord1.contents.Field.WordCnt,   \
                Msg.pCmdWord1.contents.Value))
            print
            
IrigIO.close()
