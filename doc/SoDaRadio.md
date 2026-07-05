# SoDa -- A Software Defined Radio for ADALM/Pluto, USRP, RTLSD, and ... {#mainpage}

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

SoDaRadio includes a contact logger that records time, frequency, this
grid, that grid, this call, that call.  The GUI also includes a
bearing and distance calculator for microwave users.

SoDaRadio "listens" on a socket for hamlib connections. This feature
has been used with WSJTX to operate FT8 and WSPR modes.

For information on building, installing, and using SoDaRadio see
the github pages at https://kb1vc.github.io/SoDaRadio/

There are two major parts to SoDaRadio: [the GUI](@ref ShortSoDaGUI) and 
[the server](@ref SoDaServer). The GUI is the thing that faces *you* and the 
server is the thing that faces the hardware.  The two components talk to 
each other via a set of Unix Domain Sockets (think files/pipes) over a 
clearly and kinda cleanly designed interface. 

To learn how to operate an SDR with SoDaRadio look at [the GUI](@ref ShortSoDaGUI) documentation. To learn about [hardware support]{@ref hardware_support}, [the signal processing chain](@ref how_it_works), or [how to add a new model](@ref adding_a_model) look at [the server documentation](@ref SoDaServer),

But if you just can't wait and if you have a sense of humor: 

## Quickstart

### First Steps

SoDaRadio reads configuration information (like band definitions, last
tuned frequency, volume and gain settings) from a configuration
file. Each type of radio has its own configuration file. These are
stored in `~/.config/kb1vc.org/` (`kb1vc.org` to avoid collisions with
other tools and other names. It is good manners.)  The configuration
file for a hardware model named "FReD" will be found in
`~/.config/kb1vc.org/FRED_SoDaRadioQT.conf` The model name is always
upcased. TWIG.

These files are a mite tedious to write by hand. Though you could
create a configuration file by entering in each band configuration via
SoDaRadio's "Config Band" panel, that's a little bit of a bother too.
So there's a helper program: `SoDaCreateConfig`. 

As an example, I have a PLUTO connected to a Ham-it-up transverter. The transverter LO is at (nominally) 125 MHz.  This is how I'd create a new configuration file:
```
SoDaCreateConfig --down-convert 125.0 --low-limit 70.0  PLUTO
```

I've got a USRP N200/UBX that covers DC to about 6 GHz but doesn't quite make it to 10GHz. So I add a transverter for 10 GHz. 
```
SoDaCreateConfig --up-convert 9933.0 --low-limit 10000.0 USRP
```

I've also got a USRP B210 that covers 60 MHz to 6 GHz and is connected to 
a Ham-it-up transverter for the HF bands.  So I do something like what I
did with the Pluto and with the USRP N200
```
SoDaCreateConfig --down-convert 125.0 --low-limit 70.0 --up-convert 9933.0 --low-limit 10000.0 USRPB200
```

The SoDaRadio code that manages radio models has *two* entries for USRPs. One is for the B200 series which has a lower band edge around 60 MHz or so, and the other is for "everything else."  The USRPB200 model will select our USRPB200_SoDaRadioQT.conf file. 

If you're satisfied with your newly generated configuration files, copy them to `~/.config/kb1vc.org/` and let 'er rip.

### Running SoDaRadio

Got a USRP? If it is a B2xx make sure that it is loaded and ready: 
```
uhd_usrp_probe
```
Then
```
SoDaRadio  USRPB200
```

Got an ADALM/Pluto?
```
SoDaRadio  PLUTO
```
 
How about an RTLSDR?
```
SoDaRadio  RTLSDR
```

### Running WSJTX or FLDIGI

This is harder than it ought to be. Take a look in MODEMS.md. If that file
doesn't exist, then I haven't written it yet. 

## Reporting Bugs

There's a bug somewhere in this code. And if there's one bug, you can bet there will be a bazillion others hiding with it. 

If you find one and are inclined to report it, go to the githib repo and file an issue. Your post will be most useful if you provide:
1. The distribution (Fedora? Ubuntu? Yggdrasil?) and version (43? 24.04? 0.88?)
2. The git hash of the software. Ideally, you'd just include the bill-o-materials text from the "About SoDaRadio" button on the Settings page.  Like this:
```
SoDaRadio Version: 12.1.0
Git ID: main:4d51e05
USRP Hardware Driver Version: 4.9.0
Qt Version: 6.10.3
Sources and Such: https://kb1vc.github.io/SoDaRadio/
Maintainer: kb1vc@kb1vc.org
```
3. The hardware model of the SDR.
4. A detailed description of what you heard/saw/felt. Keep it
factual. If you've got some idea of what is actually wrong, feel
free to say so. But please tell me what the manifestation of the
bug was - why were you disappointed? That's more important than the
diagnosis.


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
