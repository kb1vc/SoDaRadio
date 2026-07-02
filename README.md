# SoDa -- A Software Defined Radio for ADALM/Pluto, USRP, RTLSD, and ...

SoDaRadio is a multi-mode HF/VHF/UHF/Microwave radio transciever
that runs on Linux.   It has been tested on several 
- Ettus USRP models including the N200/WBX, N210/WBX, N210/SBX, N200/UBX,
and the B210. 
- The ADALM/Pluto 
- At least one RTLSDR dongle
  
For radios that provide TX hardware, SoDaRadio supports USB, LSB, CW,
AM, and NBFM.  A CW keyer is built-in. 

The software supports external transverters, including the Ham-it-up
converter and various microwave transverters. 

Each model of radio or family of models is driven by a three
relatively small and simple software modules. (The Pluto and RTLSDR
modules were mostly written by Claude/Sonnet 4.6.) Extending SoDaRadio
to support additional radios is likely to require a modest effort.

SoDaRadio includes a contact logger that records time, frequency, this
grid, that grid, this call, that call.  The GUI also includes a
bearing and distance calculator for microwave users.

SoDaRadio "listens" on a socket for hamlib connections. This feature
has been used with WSJTX to operate FT8 and WSPR modes.

For information on building, installing, and using SoDaRadio see
the github pages at https://kb1vc.github.io/SoDaRadio/

Packages for Ubuntu and Fedora are available somewhere near the github
page.

## Quickstart

Got a USRP? If it is a B2xx make sure that it is loaded and ready: 
```
uhd_usrp_probe
```
Then
```
SoDaRadio --radio USRP
```

Got an ADALM/Pluto?
```
SoDaRadio --radio PLUTO
```

How about an RTLSDR?
```
SoDaRadio --radio RTLSDR
```



## Copyright: 

Copyright (c) 2026 Matthew H. Reilly (kb1vc)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

(Sorry for the ALL-CAPS. But maybe shouting makes it all more legal...)

## Support for Windows

No.
