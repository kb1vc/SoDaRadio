#!/bin/bash

\section Modems SoDaRadio and External Modems (Like WSJTX and FLDIGI)

Just like with three-dimensional radios, there are two sets of connections
that need to be made between SoDa and a modem program like WSJTX: audio, 
and control. 

\subsection ModemsAudio Audio 

The audio connections, from the modem output to the SoDaRadio transmit 
chain, and from the SoDaRadio receive chain to the modem input, are 
mediated through the Linux sound system.  So the magic is nearly 
impenetrable.

There are probably lots of ways of doing this in Linux - that's part
of the charm of the steaming pile of dumpster fire that is sound support
in Linux. But here's how it is done at kb1vc:

\subsubsection ModemsPrereqs Prerequisites: 

You'll need:
* For Fedora
  - pulseaudio-utils
  - pipewire-utils
* For Ubuntu:
  - pulseaudio-utils
  - pipewire-bin

\subsubsection ModemsScript The Script

You'll need to run this script each time you start a new login session (not at boot time). It runs in user mode and creates resources that are visible to all processes with the same userid.   Place the script where you can find it. 

```
#!/bin/bash

#set up the mixed sound sink
pactl load-module module-null-sink sink_name=RadioOutput \
      sink_properties=device.description="RadioOutput"
pactl load-module module-null-sink sink_name=ModemOutput \
      sink_properties=device.description="ModemOutput"

# set up the inputs
pactl load-module module-null-sink media.class=Audio/Source/Virtual sink_name=ModemInput channel_map=front-left,front-right
pactl load-module module-null-sink media.class=Audio/Source/Virtual sink_name=RadioInput channel_map=front-left,front-right

# connect them
pw-link RadioOutput:monitor_FL ModemInput:input_FL
pw-link RadioOutput:monitor_FR ModemInput:input_FR

pw-link ModemOutput:monitor_FL RadioInput:input_FL
pw-link ModemOutput:monitor_FR RadioInput:input_FR
```

Now there will be several new devices




\subsubsection The Setup

When SoDaRadio started up, you will see several choices for an (RX)
output device.  Among them RadioOutput and ModemOutput.


Here's what I got one afternoon after a fresh boot:

* SOURCES
  *  RadioOutput.monitor	PipeWire	float32le 2ch 48000Hz	IDLE
  *  ModemOutput.monitor	PipeWire	float32le 2ch 48000Hz	IDLE
  *  ModemInput	PipeWire	float32le 2ch 48000Hz	IDLE
  *  RadioInput	PipeWire	float32le 2ch 48000Hz	IDLE
* SINKS
  * RadioOutput	PipeWire	float32le 2ch 48000Hz	IDLE
  * ModemOutput	PipeWire	float32le 2ch 48000Hz	IDLE

Selecting RX Output on the Settings panel: \image html rxsel.jpg width=400px
Selecting TX Input on the Settings panel: \image html txsel.jpg width=400px

Audio is now configured for an external modem: \image html audio_sel.jpg width=400px

\subsection SettingWSJTX Setting Up WSJTX
When I started wsjtx I went to File->Settings and saw that I
could choose from RadioOutput.monitor and others for the input port,
and ModemOutput for the output port.

This is what I ended up with: \image html wsjt_audio.jpg width=600px

Then I filled in the control fields: \image html wsjt_control.jpg width=600px

\subsection SettingFLDIGI Setting Up FLDIGI

For FLDIGI I selected Configure->ConfigureDialog and was off to the races: 


Audio is configured as a "soundcard" (remember those?): \image html fldigi_audio.jpg width=600px

Then I filled in the control fields: \image html fldigi_control.jpg width=600px

Not much to it other than that.  You may find the gain adjustments need a little fidling. It's a hobby. 
