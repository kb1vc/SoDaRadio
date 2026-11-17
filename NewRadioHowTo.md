# How to Create a New Radio Module for SoDaRadio

Adding support for new hardware requires implementing four C++ classes
and touching five files. This document walks through each step using
the PLUTO support as a worked example. When in doubt, read
`src/PlutoCtrl.cxx`, `src/PlutoRX.cxx`, `src/PlutoTX.cxx`, and
`src/PlutoRadio.cxx` — they are the simplest complete implementation
in the tree.

## Prerequisites

To make all this work, you'll need 
- a name for your radio model.  We'll call your new SDR device "Fred."
- a library that knows how to talk to a Fred. It can be called anything, 
but we'll assume it is in a package called FredLib.
- a little patience
- documentation for the library API. 
- a Fred radio. (You'll need this to test and debug stuff.)

---

## Overview

Every radio module is a set of three threads plus an aggregate
container. Remember, we're calling your radio "Fred."  If you use
another name (please do) replace 'FredCtrl' with 'yournameCtrl' and so
on.

| Class | Base class | Purpose |
|---|---|---|
| `FredCtrl` | `SoDa::RadioControl` | Tunes LOs, sets gain, controls TX/RX relay |
| `FredRX` | `SoDa::RadioRX` | Streams I/Q samples from hardware into the IF pipeline |
| `FredTX` | `SoDa::RadioTX` | Streams I/Q samples from the baseband modulator to hardware |
| `FredRadio` | `SoDa::Radio` | Creates the three threads; registered in the factory |

The server (`src/SoDaServer.cxx`) calls a factory to construct
`FredRadio`, wires mailboxes, then calls `init()` on the aggregate
before starting any threads. All post-construction communication is
via mailboxes — no direct calls between threads.  

But you don't have to know anything about factories and threads and
mailboxes and all that. All you have to do is to write the code for
the four classes: FredCtrl, FredRX, FredTX, and FredRadio. Then let 
the other modules know about the new radio. 

---

## Step 1 — Write the Control Thread (`FredCtrl`)

The control thread owns all direct hardware I/O for frequency, gain, and
TX/RX switching. It inherits `SoDa::RadioControl` (`src/RadioControl.hxx`),
which provides the `run()` loop and command dispatch. You supply the
hardware-specific implementations of these pure virtual methods:

```cpp
// LO locking state
virtual bool isLOLocked(SoDa::RXTX rxtx) = 0;

// Antenna selection
virtual std::vector<std::string> listAntennas(SoDa::RXTX rxtx) = 0;
virtual void        setAntenna(const std::string & ant, SoDa::RXTX rxtx) = 0;
virtual std::string getAntenna(SoDa::RXTX rxtx) = 0;

// Sample rate
virtual void  setSampleRate(float rate, SoDa::RXTX rxtx) = 0;
virtual float getSampleRate(SoDa::RXTX rxtx) = 0;

// Gain  (range: -30 dB to 0 dB; 0 = maximum gain / maximum power)
virtual float setRFGain(float gain, SoDa::RXTX rxtx) = 0;

// LO frequency
virtual double setLOFreq(double freq, SoDa::RXTX rxtx) = 0;
virtual double getLOFreq(SoDa::RXTX rxtx) = 0;

// TX switching
virtual bool getTXEna() = 0;
virtual bool getTXRelayOn() = 0;
virtual void setTXEna(bool tx_on, bool full_duplex) = 0;

// Identity
virtual std::string getHardwareDescription() = 0;

// Required boilerplate — returns shared_ptr to self
virtual RadioControlPtr getSelfPtr() = 0;
```

Optionally override:

```cpp
virtual void init() { }                          // hardware setup before threads run
virtual bool setClockSource(Command::ClockSource src);
virtual Command::ClockSource getClockSource();
```

### Required boilerplate in the class definition

The constructor must be `protected`, and the class must expose a static
`make` factory method that handles the `self` weak pointer and thread
registration:

```cpp
class FredCtrl : public SoDa::RadioControl {
protected:
  FredCtrl(ParamsPtr params);
public:
  ~FredCtrl();

  static FredCtrlPtr make(ParamsPtr params) {
    auto ret = std::shared_ptr<FredCtrl>(new FredCtrl(params));
    ret->self = ret;
    ret->registerThread(ret);
    return ret;
  }

  RadioControlPtr getSelfPtr() override { return self.lock(); }

  // ... all pure-virtual overrides ...

private:
  std::weak_ptr<FredCtrl> self;
};
```

If you're not familiar with "smart pointers" in C++ don't be frightened. 
Follow the pattern and all will be well. 

### Notes on `setTXEna`

This method is more important than it looks. When `tx_on` is `false` the
hardware transmitter **must** be silenced — not just muted in software.
If the hardware has no relay GPIO (like the Pluto), apply maximum
attenuation. Test thoroughly: verify the transmitter goes quiet when the
GUI exits, crashes, or the process is killed.  The Pluto, for instance, 
can be left in transmit mode after its controlling process exits unless
things are cleaned up properly on exit. 

---

## Step 2 — Write the RX Thread (`FredRX`)

The RX thread inherits `SoDa::RadioRX` (`src/RadioRX.hxx`). The base class
handles all command dispatch and IF NCO tuning. You supply:

```cpp
// Set the IF (baseband) NCO frequency
virtual void setNCOFreq(double freq) = 0;

// Pull one block of samples from hardware, beat against NCO, publish to if_stream.
// Return true if samples were available.
virtual bool downConvert() = 0;

// Start/stop hardware sample streaming
virtual void startStream() = 0;
virtual void stopStream()  = 0;
virtual bool streamEnabled() = 0;

// Gate the raw RF sample path to the periodogram (waterfall)
virtual void enableIFStreamer(bool enable) = 0;
```

Optionally override `init()` if the hardware DMA buffers must be allocated
after `FredCtrl::init()` has configured sample rates (the Pluto requires
this ordering: ctrl → rx → tx in `FredRadio::init()`).

The `PlutoRX` and `RTLSDRRx` implementations were AI-generated from
`USRPRX` as a template and required minimal human correction — start there.

---

## Step 3 — Write the TX Thread (`FredTX`)

The TX thread inherits `SoDa::RadioTX` (`src/RadioTX.hxx`). You supply:

```cpp
// Enable or disable the hardware transmit chain
// Return true if unit was previously in transmit mode.
virtual bool transmitSwitch(bool tx_on) = 0;

// Push one buffer of complex samples to hardware (may block).
// Return true if the buffer was sent directly (not queued).
virtual bool put(CBufPtr buf) = 0;
```

Optionally override `init()` and `subExecSetCommand()` / `subExecRepCommand()`
if the TX thread needs to react to extra commands.

---

## Step 4 — Write the Aggregate (`FredRadio`)

This is a thin container. Create the three threads in the constructor and
call their `init()` methods in the right order from `Radio::init()`:

```cpp
// FredRadio.hxx
#include "Radio.hxx"
#include "FredCtrl.hxx"
#include "FredRX.hxx"
#include "FredTX.hxx"

namespace SoDa {
  class FredRadio : public Radio {
  protected:
    FredRadio(ParamsPtr params);
  public:
    static RadioPtr make(ParamsPtr params) {
      return std::shared_ptr<Radio>(new FredRadio(params));
    }
    void init();
  private:
    FredCtrlPtr ctrl;
    FredRXPtr   rx;
    FredTXPtr   tx;
  };
}

// FredRadio.cxx
FredRadio::FredRadio(ParamsPtr params) : Radio(params) {
  ctrl = FredCtrl::make(params);
  rx   = FredRX::make(params);
  tx   = FredTX::make(params);
}

void FredRadio::init() {
  ctrl->init();
  rx->init();
  tx->init();
}
```

If the RX and TX threads need a hardware handle from the control thread
(as `USRPRX`/`USRPTX` need the UHD `multi_usrp` object), pass it as an
extra argument to their `make` calls and create any widgets that need to
be shared in the 'FredRadio' module. 

---

## Step 5 — Register in `src/RadioModels.cxx`

This is the **only** file in `src/` you need to touch besides your new files.
Add a `#include` for your aggregate header and one line in `RadioModels::init()`:

```cpp
// At the top, inside the #if guard for your hardware:
#if HAVE_FRED
#include "FredRadio.hxx"
#endif

// Inside RadioModels::init():
#if HAVE_FRED
  model_map["FRED"] = FredRadio::make;
#endif
```

The key string (`"FRED"`) is what the user passes as the radio type
argument on the command line.

---

## Step 6 — Update `common/GuiParams.cxx`

The GUI help text lists the available radio types. Add your type name inside
a matching `#if` guard so it only appears when the hardware was compiled in:

```cpp
#if HAVE_FRED
  cmd.addInfo("\tFRED");
#endif
```

This block is in `GuiParams::GuiParams()` just before the `cmd.parse()` call.
A comment in the file notes this must stay in sync with `RadioModels.cxx` and
`CMakeLists.txt`.

---

## Step 7 — Update `src/CMakeLists.txt`

Add a CMake detection block and wire it to the server build:

```cmake
# --- find / enable your hardware library ---
find_package(MyRadioLib QUIET)
if(MyRadioLib_FOUND)
  set(HAVE_FRED 1)
  add_definitions(-DHAVE_FRED=1)
else()
  set(HAVE_FRED 0)
endif()

# --- add sources when available ---
IF(${HAVE_FRED})
  LIST(APPEND SoDaServer_SRCS
    FredCtrl.cxx
    FredRX.cxx
    FredTX.cxx
    FredRadio.cxx
  )
  LIST(APPEND Radio_INCLUDE_DIRS ${MyRadioLib_INCLUDE_DIRS})
  LIST(APPEND Radio_LIBRARIES    ${MyRadioLib_LIBRARIES})
ENDIF()
```

Look at the `HAVE_PLUTO` or `HAVE_RTLSDR` blocks immediately above for the
exact pattern already used in the file.

---

## Files to create / modify — summary

| Action | File |
|---|---|
| Create | `src/FredCtrl.hxx`, `src/FredCtrl.cxx` |
| Create | `src/FredRX.hxx`, `src/FredRX.cxx` |
| Create | `src/FredTX.hxx`, `src/FredTX.cxx` |
| Create | `src/FredRadio.hxx`, `src/FredRadio.cxx` |
| Edit — add `#include` + one line in `init()` | `src/RadioModels.cxx` |
| Edit — add `addInfo` line | `common/GuiParams.cxx` |
| Edit — add `find_package`, `IF` block | `src/CMakeLists.txt` |

That last part - the CMakeLists.txt stuff - can be the most
frustrating.  If you can follow the pattern in the existing
CMakeLists.txt file for the libIIO library you are home free. Just
crib that code and make a copy of cmake/Modules/BuildLibIIO.cmake for
your library. (Wait! Don't panic!  See the next section.)


---

## Testing checklist

Before submitting or merging:

- [ ] Radio tunes, receives, and (if applicable) transmits correctly.
- [ ] Exiting the GUI shuts off the transmitter. Verify with a power meter
  or spectrum analyser — do not rely on software state alone.
- [ ] Starting the GUI a second time after a clean exit works without
  requiring a hardware power-cycle.
- [ ] `SoDaRadio FRED --help` exits cleanly with no crash.
- [ ] Build succeeds with the hardware library absent (`-DHAVE_FRED=0`
  or the library simply not installed); all other radio types still work.

---

## Submitting to the main tree

See `README.md` for the project's contribution policy. In brief:

- Include a description of the hardware (model, manufacturer, version) and
  the required library (name, version, where to get it).
- If the hardware is a one-off or prototype, keep it on a branch in your
  fork rather than submitting it upstream.
- Hardware vendors are welcome to send a loaner unit to kb1vc@kb1vc.org for
  integration testing.


# Let Claude Do It. 

Not-so-dirty little secret: I love writing code. But don't like
wandering through the goat entrails that pass for API documentation of
some radio libraries. I wrote the USRP models all by myself with
nothing but the USRP library documentation, a couple of USRPs, and a
love for C++ (and an Ettus Application Engineer on speed-dial). But I
really didn't want to spend a lot of time learning the ins-and-outs of
the ADALM/Pluto and its use of libiio. Life is too short. So I let
Claude/Sonnet do it.

I really really dislike creating new CMake magic. But we need CMake 
magic to make all the library dependencies work. Without it we have to 
count on future users to download all kinds of dependencies and untangle
nightmares too horrible to contemplate.  So I had Claude/Sonnet do that 
too. That's why the 'cmake/Modules/Build*.cmake' files don't look like a
real human wrote them. 

So, how can *you* do this? (BTW, this would probably work with any of the 
other AI widgets; Claude isn't that special.) Let's take "AirSpy" as an example. Here's the prompt I wrote: 

> Please add support for the airspy radio to SoDaRadio.
> 
> Create src/AirSpyCtrl.[ch]xx src/AirSpyRX.[ch]xx src/AirSpyTX.[ch]xx
> src/AirSpyRadio.[ch]xx and add the necessary parts to
> src/RadioModels.[ch]xx and common/GuiParams.[ch]xx
> 
> This will depend on libairspy found in the git repository at
> https://github.com/airspy/airspyone_host.git
> 
> Create a cmake module BuildAirSpy.cmake to fetch and build the library code.
> 
> Link the code in as a static library.
> 
> Modify the toplevel CMakeLists.txt file and the src/CMakeLists.txt file. 
> 
> All the documentation on airspy is in the source code in its kit
> airspyone_host/libairspy/src/* Read that for insight on what you'll
> need to build the support.
> 
> You may find looking at the support for RTLSDR and Pluto helpful.
> 
> Mark any files that you create or edit with a statement "Created whole
> or in part by " and then insert your llm model name and version.
> 
> Use git commit fc61ab6be57ed61f0e2bdd9c6dfae74cacef57d0 in the airspy
> repo. This may not be the head of the master branch, so you may need
> to clone the repo as "unshallow"

Claude cranked and cranked for about 6 minutes and produced a cut at
all the required files. The build failed the first time, but I asked
Claude to look at the log and fix the problems. It apparently was
"guessing" at a few function names.  That happens. Treate any of these
AI things like a  very eager and bright and demented intern.
Claude corrected the errors and the second build succeeded.

I specified a particular git hash for the build because the airspy folks
don't tag their releases. In fact, it appears that they only publish
the 'master' branch of their repo. It is all a matter of taste, but I
often specify a particular version if I need to import a library to avoid
surprises. 

Building is one thing, working is another.  Claude's code isn't nearly
as readable and consistent as stuff written by a good programmer.  It
is also wrong at times. Claude/Sonnet can trip over real-time
requirements, You will likely spend some debug time trying to get
buffer flow control and timing to work. This is harder than they
present it in the movies. If you're building a transceiver, watch out
for buffer under-runs. These will cause gaps in the transmitted signal
that will sound like clicks. They'll also screw up any digital
modulation you might want to
support.  

I don't have an AirSpy radio, so building the code was as
far as I could go with this experiment. But you might own one. Or a
FizBlatz 3000 SDR.  Try building your own support. If it works, submit
a pull request and we can discuss integrating it into the SoDaRadio
repository.

