
# Starting SoDaRadio

In 2012 I began work on an IF radio to drive a 10GHz transverter
system.

The previous radio -- an FT817 -- had worked well, but it was time for
a change. This provided an excuse to try out the USRP/N200 software
defined radio system manufactured by Ettus Research. It was also the
impetus to develop SoDa, the software defined radio application for
the N200.

The goals of the SoDa project were modest:
* Function as a ``learning lab'' to explore software defined radio and
digital signal processing concepts.
* Provide a practical platform for interfacing to a microwave transverter.
* Create a versatile all-mode exciter for the VHF and UHF amateur bands.
* Improve on the performance of my earlier microwave systems that used an ``analog'' IF radio.
 
Over time, SoDaRadio sprouted a bunch of features -- assisted logging
(especially for the ARRL 10GHz and Up Cumulative Contest), GPS
monitoring, bearing and distance calculation, and finally a hamlib
network interface.

In the summer of 2017, I abandoned the original GUI, based on Wx,
since the support for Wx on Ubuntu was beginning to rot.  The new GUI,
based on Qt, is simpler, and should be much more capable and far less
fiddly. Time will tell.

The most exciting addition with version 5.0.0 is the hamlib
interface. SoDaRadio can now talk to WSJT-X and FLDIGI.  On September
9, 2017, SoDaRadio and WSJT-X completed a contact between kb1vc and
wu2m on the 40m band using FT8.  SoDaRadio was running at kb1vc on an
Ettus N200 with a UBX module.  Power to the Outbacker Outreach
vertical antenna was about 14dBm.  That's more than 11000 miles per
watt.

In September of 2017, the SoDaRadio project moved from Sourceforge
to GitHub.


