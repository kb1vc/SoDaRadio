# SoDa -- A Software Defined Radio for the Ettus Research USRP

SoDaRadio is a multi-mode VHF/UHF/Microwave radio transciever
that runs on Linux.   It has been tested on several Ettus
USRP models including the N200/WBX, N210/WBX, N210/SBX, N200/UBX,
and the B210. 

It looks like this: 
![Image of SoDaRadio tuned to 20m band](/images/SoDaRadio_20m.png)

It is non-commercial and released under the BSD 2-Clause license. 

SoDaRadio does not yet run on any other hardware platform. 

SoDaRadio has been built on both Fedora and Ubuntu releases.  There is no
particular reason that it couldn't run on any Linux release that is clean 
enough to compile and run the UHD utilities that come with the Ettus software.

Interested in more detail?  Take a look at the [documentation here](docs/index.html).  ["SoDa -- A Multimode VHF/UHF Software Defined Radio"](papers/SoDa_EasternVHF_2014.pdf) was presented at the 40th Eastern VHF/UHF Conference in 2014.

Interested in how SoDaRadio got [here?](History.md) 

## Getting SoDaRadio

You have two choices here.  

You may build SoDaRadio from scratch using git
and cmake.  This really does work rather smoothly on Fedora and Ubuntu systems. 
Before each major release, I do a build on at least two recent Fedora releases, 
and the current and previous Ubuntu LTS releases. 

Ready to build-it-yourself?  Jump down to "Building from scratch."

Alternatively, if you are running Ubuntu 16.04, Fedora 25, or Fedora 24, you can
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

This process has been tested on straight-out-of-the-box Fedora 25 and Fedora 24
releases.  It installs all the prerequisites.  (If you really need to use the 
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

#### Ubuntu prerequisites

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
git clone https://git.code.sf.net/p/sodaradio/SoDaRadio/
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
it to radiogeek381 at gmail.com  with the output of the folliwing command: 
~~~~
git rev-parse HEAD
~~~~
The command must be typed somwhere in the tree that you just created with
the git clone command. 


### Installing from a kit

Kit installs are pretty simple.  Download the appropriate package, and
install with the normal installer for your OS. If you've managed to install 
the Ettus software, then SoDaRadio should not present any particular problems.

#### Fedora

The following works for installing on Fedora 25.  Fedora 24 is identical 

1. Download the appropriate rpm. The following links will take you to the repository 
location for the kit. Go to that page and hit the download button.
   * [Get SoDaRadio for Fedora 25 here](/packages/rpm/SoDaRadio-5.0.0-1.x86_64.Fedora-25.rpm)
   * [Get SoDaRadio for Fedora 24 here](/packages/rpm/SoDaRadio-5.0.0-1.x86_64.Fedora-24.rpm)
1. Install the kit using dnf (substitute the correct package name in the command below).
~~~~
sudo dnf install ./SoDaRadio-5.0.0-1.x86_64.Fedora-25.rpm
~~~~


#### Ubuntu

SoDaRadio builds and runs on Ubuntu 14.04, but the deb package for it does not work for some mysterious reason. 

The kit for Ubuntu 16.04, however, appears to work just fine.  

1. Download the deb file.  The following link will take you to the repository location
for the kit.  Go to that page and hit the download button.  [Get SoDaRadio for Ubuntu 16.04 here](/packages/deb/SoDaRadio-5.0.0-1.x86_64.Ubuntu-16.04.deb)
2. Install the kit using apt
~~~~
sudo apt install ./SoDaRadio-5.0.0-1.x86_64.Ubuntu-16.04.deb
~~~~

## Using SoDaRadio

To start the SoDaRadio, connect your Ettus USRP to 
the host computer. Connect an antenna to the TX/RX port.

Then execute SoDaRadio

~~~~~
SoDaRadio
~~~~~

If you haven't installed SoDaRadio, but have built it from scratch,
you can start SoDaRadio from the build directory like this: 
~~~~~
qtgui/SoDaRadio --server=src/SoDaRadio 
~~~~~

It should come up tuned to something like 10,368.100 or nearby.
That's not a very good default.  Sorry about that. To tune to a
more reasonable frequency, slide your mouse over the RX frequency
numbers.  The "underbar" that sits beneath one of the digits indicates
that this is the digit that your mouse can change.  Click MB1 above the
middle of the numbers to go up, below to go down.  To move to a new
digit, click MB3 on the right half of the number display to go down, 
left half to go up.  Yes this is crap UI design.  I'll fix it soon.

Once you've selected a ferquency, hit the "Center RX Freq" button 
on the waterfall display.  That will center your chosen frequency in 
the waterfall window.  After that, you can click on the waterfall to 
move the RX freqeuncy.  

I often click the "TX/RX Lock" button, since I don't spend a whole lot
of time working split.  

Poke buttons.  Try things.  Don't hit the "TX" button unless
you have something connected and the necessary license to operate
in that band. 

