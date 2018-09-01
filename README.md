# SoDa -- A Software Defined Radio for the Ettus Research USRP

## New! Improved!!! (See Version News below.)


SoDaRadio is a multi-mode VHF/UHF/Microwave radio transceiver
that runs on Linux.   It has been tested on several Ettus
USRP models including the N200/WBX, N210/WBX, N210/SBX, N200/UBX,
and the B210. 

It looks like this: 
![Image of SoDaRadio tuned to 20m band](/images/SoDaRadio_20m.png)

It is non-commercial and released under the BSD 2-Clause license. 

SoDaRadio does not yet run on any other hardware platform. 

SoDaRadio has been built on both Fedora 28 and Ubuntu 18.04 releases.  There is no
particular reason that it couldn't run on any Linux release that is clean 
enough to compile and run the UHD utilities that come with the Ettus software and that supports Qt5.7 or later. 

Interested in more detail?  Take a look at the [documentation here](docs/index.html).  ["SoDa -- A Multimode VHF/UHF Software Defined Radio"](papers/SoDa_EasternVHF_2014.pdf) was presented at the 40th Eastern VHF/UHF Conference in 2014.

Interested in how SoDaRadio got [here?](History.md) 


## Getting SoDaRadio

You have two choices here.  

You may build SoDaRadio from scratch using git
and cmake.  This really does work rather smoothly on Fedora and Ubuntu systems. 
Before each major release, I do a build on at least two recent Fedora releases, 
and the current and previous Ubuntu LTS releases. 

Ready to build-it-yourself?  Jump down to "Building from scratch."

Alternatively, if you are running Ubuntu 16.04, or Fedora 27 you can
download binary kits that just "do the right thing."  They'll install all the 
prerequisites, put the necessary files in the right place and are generally a 
pain-reduced way of getting SoDaRadio running.  The only real downside to the binary
installation is if you really really must run something other than the libuhd version
that is supplied by your distribution (if you use Fedora) or by Ettus (if you use
Unbuntu).  This can be a showstopper for folks who's GnuRadio installation is 
finicky about which version of libuhd it needs.  If you think you may be one of those
people, build SoDaRadio from scratch. 

If you're OK with using your distributions libuhd release, install from a 
binary kit. See the
binary kit installation instructions below, at "Installing from a kit."

### Building from scratch

Sources can be cloned or downloaded from the github site.  If you haven't used
git, don't get wrapped around the axle.  You won't need mad wizard ski112 to 
get through this. 

The next steps depend on which distribution you are using. 

#### Fedora prerequisites

This process has been tested on a straight-out-of-the-box Fedora 27 release.
It installs all the prerequisites.  (If you really need to use the 
libuhd version that you've installed yourself for some other reason, skip the
last installation step.)

Run the installation commands as "root" or use sudo.  
~~~~~
 dnf group install --assumeyes "C Development Tools and Libraries"
 dnf install --assumeyes libusb-devel doxygen boost-devel
 dnf install --assumeyes fftw-devel alsa-lib-devel
 dnf install --assumeyes qt5-qtbase-devel
 dnf install --assumeyes qwt-qt5-devel
 dnf install --assumeyes python-mako
 dnf install --assumeyes qt5-qtmultimedia-devel
 dnf install --assumeyes hamlib-devel
 dnf install --assumeyes gpsd-devel gpsd-clients gpsd-libs gpsd
 dnf install --assumeyes rpm-build
 dnf install --assumeyes git
 dnf install --assumeyes redhat-lsb
 # Leave this one out if you really need to use some other version of libuhd
 dnf install --assumeyes uhd-devel uhd-firmware
~~~~~

Now it is time to get a copy of the sources and build the kit.  That
process is pretty much identical for all of the distributions, so continue on to
"Compiling and installing on any platform"

#### Ubuntu prerequisites (Ubuntu 18.04 required)

# SoDaRadio no longer builds on Ubuntu 16.04 -- its Qt5 version is far out of date
(This is somewhat frustrating, because the last update to 16.04 was more than a year and a half after the required version of Qt5 came out.)

Run the installation commands as "root" or use sudo.  
~~~~~
add-apt-repository ppa:ettusresearch/uhd
apt-get update
apt-get install -y libhamlib-dev
apt-get install -y libqwt-qt5-dev
apt-get install -y libqt5widgets5
apt-get install -y libqt5core5a
apt-get install -y qtbase5-dev qt5-qmake qtbase5-dev-tools
apt-get install -y libqt5multimedia5
apt-get install -y qtmultimedia5-dev
apt install -y debmake
apt install -y dpkg-dev
apt install -y cmake
apt install -y libboost-all-dev libasound2-dev libfftw3-dev
apt install -y pkg-config
apt install -y gpsd gpsd-clients libgps-dev
apt install -y git
# You'll want to skip the next two commands
# if you have your own special libuhd version.
apt-get install -y libuhd-dev uhd-host 
/usr/lib/uhd/utils/uhd_images_downloader.py
~~~~~


#### Compiling and installing on *any* platform

~~~~~
git clone https://github.com/kb1vc/SoDaRadio.git
cd SoDaRadio
mkdir build
cd build
cmake  ../
make 
sudo make install
~~~~~
This will run for a while.  You should see *absolutely no errors or warnings*
as I put real effort into making sure the system builds cleanly.  If you get a
warning or error, copy the text of the warning/error into an email and send 
it to radiogeek381 at gmail.com  with the output of the following command: 
~~~~
git rev-parse HEAD
~~~~
The command must be typed somewhere in the tree that you just created with
the git clone command. 


### Installing from a kit

Kit installs are pretty simple.  Download the appropriate package, and
install with the normal installer for your OS. If you've managed to install 
the Ettus software, then SoDaRadio should not present any particular problems.

#### Fedora

The following works for installing on Fedora 27. Older kits for Fedora 26, 25, and 24
are no longer supported. Version 5.3 fixes significant problems with T/R switching
in older versions. 

1. Download the appropriate rpm. The following links will take you to the repository 
location for the kit. Go to that page and hit the download button.
   * [Get SoDaRadio V6.0.1 for Fedora 28 here](/packages/rpm/SoDaRadio-6.0.1-1.x86_64.Fedora-28.rpm)
   * [Get (obsolete version of) SoDaRadio for Fedora 27 here](/packages/rpm/SoDaRadio-5.3.0-1.x86_64.Fedora-27.rpm)
   * [Get (obsolete version of) SoDaRadio for Fedora 26 here](/packages/rpm/SoDaRadio-5.3.0-1.x86_64.Fedora-26.rpm)
   * [Get (obsolete version of) SoDaRadio for Fedora 25 here](/packages/rpm/SoDaRadio-5.0.3-1.x86_64.Fedora-25.rpm)
   * [Get (obsolete version of) SoDaRadio for Fedora 24 here](/packages/rpm/SoDaRadio-5.0.3-1.x86_64.Fedora-24.rpm)
2. Install the kit using dnf (substitute the correct package name in the command below).
~~~~
sudo dnf install ./SoDaRadio-6.0.1-1.x86_64.Fedora-28.rpm
~~~~

Version 6.0.1 fixes some audio problems with 5.3 and more modern distros.  For reasons that are 
beyond my ken, 5.3 stopped behaving well soon after an upgrade to Fedora 28.  Others have reported 
similar problems on Ubuntu.  (The audio output stops after a period of time ranging from seconds to days.)
This has been addressed in version 6. 

Version 5.3 corrects some very nasty bugs in the earlier release.  In particular
T/R switching events could cause the ALSA audio output to lock up or stammer. This
has since been fixed.  (See the git logs for version soda-5.3.0 )

#### Ubuntu

SoDaRadio is no longer supported on Ubuntu 16.04, 17.10 or prior releases.
None of the older releases have adequate support for Qt. SoDaRadio versions prior to 6.0 may build on Ubuntu systems. 

1. Download the deb file.  The following link will take you to the repository location
for the kit.  Go to that page and hit the download button.
[Get SoDaRadio for Ubuntu 18.04 here](/packages/deb/SoDaRadio-6.0.3-1.x86_64.Ubuntu-18.04.deb
2. Install the kit using apt
~~~~
sudo apt install ./SoDaRadio-6.0.3-1.x86_64.Ubuntu-18.04.deb
~~~~

Please report success/failure to the github issues discussion.

## Using SoDaRadio

To start the SoDaRadio, connect your Ettus USRP to 
the host computer. Connect an antenna to the TX/RX port.

For users of the B2xx series, it is best to initialize the radio with uhd_usrp_probe. Firmware loading can take some time, and SoDaRadio can't tell whether the radio is slowly initializing, or the control link is broken.  

Then execute SoDaRadio

~~~~~
SoDaRadio
~~~~~

If you haven't installed SoDaRadio, but have built it from scratch,
you can start SoDaRadio from the build directory like this: 
~~~~~
qtgui/SoDaRadio --server=src/SoDaServer
~~~~~

It should come up tuned to something like 10,368.100 or nearby.
That's not a very good default.  Sorry about that. To tune to a
more reasonable frequency, slide your mouse over the RX frequency
numbers.  The "underbar" that sits beneath one of the digits indicates
that this is the digit that your mouse can change.  Click MB1 above the
middle of the numbers to go up, below to go down.  To move to a new
digit, click MB3 on the right half of the number display to go down, 
left half to go up.  Yes this is crap UI design.  I'll fix it soon.

Once you've selected a frequency, hit the "Center RX Freq" button 
on the waterfall display.  That will center your chosen frequency in 
the waterfall window.  After that, you can click on the waterfall to 
move the RX frequency.  

I often click the "TX/RX Lock" button, since I don't spend a whole lot
of time working split.  

Poke buttons.  Try things.  Don't hit the "TX" button unless
you have something connected and the necessary license to operate
in that band. 

## [VersionNews]: Version News

Version 6.0.0 takes a new approach to handling Audio Output

1. The GUI now has an audio listener that collects audio buffers from
the radio and forwards them to a Qt AudioOutput object.  Qt is much
better at managing the underlying ALSA audio device than I am.

2. The GUI now allows the user to choose any of the available audio output
devices. So users who want to link SoDaRadio to fldigi or wsjtx can create
virtual audio devices and connect through them. (I'll update the wiki with
an easy recipe for this on Fedora...)

3. The transmit audio chain is still on the SoDaServer side.  This will
eventually migrate to the GUI as well.  For now, there is a picker that
chooses a TX audio device, but it has no effect. 

The move to an asynchronous audio chain through QAudioOutput has eliminated
the instability seen with Version 5.3.


Version 5.3.0 has a couple of new features:

1. It is no longer horribly broken.  Previous revisions were subject to
a race between restarting an ALSA audio output stream and stuffing the
first samples into the stream.  For reasons that I can't fathom, none of
the guidance that I found on ALSA for restarting or resetting a stream
that had run dry would work.  So, v5.3.0 reworks the TR switching on the
receiver to eliminate the race.  Now SoDa radio never stops the audio
stream once it has been started.

2. The "Carrier" toggle button now actually sends a dead carrier.
This is handy for tuning and for other things. (See item 5.)

3. There is now a "turn on the external 10MHz reference" button in the
SETUP tab.  This allows the USRP to use an external 10MHz reference to
drive the USRP oscillators.  I use a Trimble Thunderbolt that stays on
all the time.  This is a huge improvement over the internal master
oscillator.

4. As an experiment, I've added a WSPR filter choice.  I don't think it
is all that great, and it has some unintended side-lobes.  (Normally, I
use the "PASS" filter for WSJT.)

5. Recently, the Ebay Angels delivered a Tek 492 spectrum analyzer to the
shack.  The Notes.txt file in the source blob has the results of some
measurements of output from a UBX module.

