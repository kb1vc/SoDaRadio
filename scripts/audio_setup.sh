#!/bin/bash

# There are probably lots of ways of doing this in Linux - that's part
# of the charm of the steaming pile of dumpster fire that is sound support
# in Linux. But here's how it is done at kb1vc:
#
# You'll need:
#  For Fedora
#     pulseaudio-utils
#     pipewire-utils
#
#  For Ubuntu:
#     pulseaudio-utils
#     pipewire-bin
#
# 

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

# Now there will be several new devices
# Here's what I got one afternoon after a fresh boot:
# SOURCES
#  RadioOutput.monitor	PipeWire	float32le 2ch 48000Hz	IDLE
#  ModemOutput.monitor	PipeWire	float32le 2ch 48000Hz	IDLE
#  ModemInput	PipeWire	float32le 2ch 48000Hz	IDLE
#  RadioInput	PipeWire	float32le 2ch 48000Hz	IDLE
# SINKS
#  RadioOutput	PipeWire	float32le 2ch 48000Hz	IDLE
#  ModemOutput	PipeWire	float32le 2ch 48000Hz	IDLE

# When SoDaRadio started up, I had several choices for an (RX) output device.
# Among them RadioOutput and ModemOutput.
#
# I had several choices for a (TX) input device.
# Among them RadioInput and ModemInput
#
# When I started wsjtx I went to File->Settings->Audio and saw that I
# could choose from RadioOutput.monitor and others for the input port,
# and ModemOutput for the output port.
#
# For FLDIGI I selected Configure-> ->Soundcard->Devices and chose
# "Port Audio" and set Capture to "RadioOutput" and Playback to
# "ModemOutput"
#
#
# Co-incidentally, for WSJTx I setup the radio control stuff like this:
# File->Settings->Radio
#  Rig: Hamlib NET rigctl
#  Network server; 127.0.0.1:4575
# The rest doesn't really matter, but I set baud rate to 4800,
# default data bits and stop bits, default handshake, PTT method CAT,
# port /dev/ttyUSB0 - which won't matter. Transmit Audio Source: front/mic
# Mode: None Split Operation None

# FLDIGI
# Configure-> ->Rig Control -> Hamlib
# Use Hamlib: check. 
# Rig: Hamlib NET rigctl(Stable)
# Device: 127.0.0.1:4575

