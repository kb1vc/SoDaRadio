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

## Getting the Program(s)

### Install From a Kit

This process ought to be simple.  If it isn't, file a bug. 

#### Fedora

First get the kit [Download RPM (Fedora)](https://github.com/kb1vc/SoDaRadio/releases/latest/download/SoDaRadio-12.3.0-1.x86_64.rpm) 

Then 
```
sudo dnf install SoDaRadio-12.3.0-1.x86_64.rpm
```

Note the version number may change -- whatever you get for the RPM, you
should use that in the dnf install. 

#### Ubuntu

First get the kit: [Download DEB (Ubuntu)](https://github.com/kb1vc/SoDaRadio/releases/latest/download/sodaradio_12.3.0_amd64.deb)

Then 
```
sudo apt install ./sodaradio_12.3.0_amd64.deb
```

Note the version number may change -- whatever you get for the deb package, you
should use that in the dnf install. 



### Building From Source

There is a set of dependencies for SoDaRadio. Most are vanilla packages
that should be available in any distribution. 
- cmake: That's how it is built. 
- make: That's how it is built. 
- pkgconf: The cmake system relies on pkgconf to find some dependencies.
- Threads: If you don't have it your c++ installation is broken. 
- Qt6: The gui framework needs Qt6Core, Qt6Network, Qt6PrintSupport, Qt6Multimedia.
- Qwt: The Qt widgets toolkit dependency can be a problem for Ubuntu users - a good argument for finding a SoDaRadio deb package. 
- readline: This is used for a command-line-interpreter widget that is useful for debugging new radio models. 
- Doxygen: Optional, but if you have it you'll get nice web pages and such. Not so important unless you want to navigate through the source code. 
- git: Without git your version strings won't get set correctly and you might have trouble reporting bugs. 
- and a few others

A few are required for particular radio models:
- libuhd: required for Ettus USRP radios. (optional)
- librtlsdr: required for inexpensive radio dongles. 
- libiio: required for the ADALM/Pluto. 
The rtlsdr lib and libiio can be built as part of the SoDaRadio build
if they haven't been installed already. The CMake options BUILD_LIBIIO
and BUILD_LIBRTLSDR enables the local build.

Two aren't quite so vanilla: 
- SoDaLibs: two libraries - sodautils and sodasignals that do lots of
housekeeping and signal processing for SoDaRadio. Will be built as part
of the SoDaRadio build if they haven't already been installed. 
- FFTW: a fast FFT library. This will be built as part of the SoDaRadio
build if it hasn't already been installed. 

#### Fedora Dependencies

From-scratch builds are tested in charliecloud instances that use this
set of install packages. The builds work. 

```
dnf -y install cmake make gcc-c++ git pkg-config 
dnf -y install python3 uhd-devel fftw-devel 
dnf -y install qt6-qtbase-devel qt6-qtmultimedia-devel qwt-qt6-devel 
dnf -y install rpm-build openssl-devel readline-devel libusbx-devel 
dnf -y install libxml2-devel flex bison
```


#### Ubuntu Dependencies


From-scratch builds are tested in charliecloud instances that use this
set of install packages. The builds work. 
```
apt-get install -y cmake make gcc g++ git pkg-config
apt-get install -y libuhd-dev libfftw3-dev
apt-get install -y qt6-base-dev qt6-multimedia-dev
apt-get install -y libcups2-dev dpkg-dev
apt-get install -y file libssl-dev libreadline-dev
apt-get install -y libusb-1.0-0-dev libxml2-dev flex bison
```

Frankly, I have no idea how flex and bison got in there. 

#### The Build Process

This is a CMake project. It works like all the others. Here's how to build
the radio wit all the trimmings. It will build the "special" pre-requisites (SoDaLibs, librtlsdr, libiio, fftw) from online sources if you don't have the libraries. I'm assuming that you've cloned the source tree and are sitting at the 
toplevel (project) directory. 
```
mkdir -p build
cd build 
cmake ../
make 
make install
```

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

### Vulnerability Reports

Sure. But please, don't bother with Claude/Sonnet/Opus/Whatever generated
reports. I can do that myself. If you really think you found something, post 
an issue (with version numbers and all that) along with the *prompt* that you used. But don't send a pull request.

### Pull Requests

I'm doing this as a hobby. Servicing pull requests isn't way high on the 
priority list. Servicing machine-generated pull requests doesn't even make
the bottom of the list. 

If you're submitting a pull request because you put together a nice
feature, thanks. Describe the new feature, tell me why it is important
to you, and provide a short summary of the new code and how it works.

If you're submitting a pull request because you found and fixed a bug,
thank you. But please keep the blast radius small. Bug fixes like "the
PTT button stuck, so I refactored all files with more than one vowel
in the name" are best kept to yourself. 

In any case, be nice. Don't reformat big swaths of code. Tabs, spaces,
yup we all have our ways, but this isn't the place to fight that
one. Don't screw with formatting: diffs are tough enough to read
without having them poluted by a whole lot of tabification crap.

### New Radio Models

I've put a lot of effort into refactoring the earlier code that was 
entirely specific to the USRP. Now there's a virtual class definition that
is a template for code to support any other model. Take a look at the 
rtlsdr support in RTLSDRCtrl, RTLSDRRX and such. 

You'll note that it is all generated by Claude/Sonnet. No kidding. This is just the kind of thing that Claude does well - crawling through a bunch of 
vague, skimpy documentation and reading source code. You can do that too. 
But if you submit a pull request with new hardware support, you should
do a few things first: 

1. Make sure it builds against a recent version on the main
branch. Please don't submit new model code based on a software
version that's a year behind the frontier.
2. Describe the hardware - model name/number, version number, manufacturer. 
3. Describe the required software - does it need its own special library? Soapy? libusb? Where does the library come from? 
4. Test it. It is especially important to ensure that the radio works
after you exit SoDaRadio and start SoDaRadio up again. For
transmitters or transceivers, make sure the transmit output shuts off
when you exit SoDaRadio.  (This was an issue with the early work on
Pluto. SoDaRadio would exit and the Pluto still blasted out its 5 mW
of carrier, destroying the RF environment for several feet.)

If the hardware is a one-off, I'd rather not include it in the main
sources. Instead, you should keep a branch in your cloned repo. 

If you are a hardware vendor, consider sending a loaner to me and I'll
do what I can and return it one way or the other. But remember, I'm doing
this as a hobby. 

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
