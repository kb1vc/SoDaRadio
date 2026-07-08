\page ShortSoDaGUI The SoDaRadio Graphical User Interface

There have been three GUIs in SoDaRadio's history. 

The first was abandoned when the framework was no longer supported on major distributions. 

The second was a Qt GUI that proved quite serviceable for several
years of competition in the "10 GHz and Up" microwave
contest. However, it required a large screen and was a little awkward
to navigate. That GUI is still built as part of the project and is
called "FlatSoDaRadio." There's [not much I want to say](@ref FlatSoDaGUI) about that. If
you've used it, knock yourself out, but I'm only keeping it around for
old-time's sake. 

The third is an evolving GUI that is meant to take up less screen real-estate and may be suitable for very small (say 9-inch) screens as one might find on a RaspberryPi. That's the interface we'll talk about here.

\section gui_basics The GUI Presentation 

The GUI is designed to be simple, present discrete chunks of information with minimal extraneous clutter. Did the design hit the mark?  No. But it is better than some, and certainly better than FlatSoDa. 

The interface is divided into panels. There is only one popup and it
is for display only - it has no buttons. The panels are: 
1. -Waterfall- The first two panels present the most common views for
receiver tuning and signal finding. Waterfall tends to be more useful
to me, so it is first. It shows a heat map of power spectral density
(in color) vs. frequency and time. Often weak signals will manifest
themselves in small changes in color.

1. -Periodogram- The periodogram window can help in peaking signals or
getting a clearer view of stuff vs time. A two-d graph with power
spectral density on the Y axis and frequency on the X axis.
2. Transmit
3. Settings
4. Band Configuration
5. Log Editor
6. Status 


\subsection commonControls Common Controls

Several controls appear on multiple panels. 
1. -RX Frequency- and -TX Frequency-  (Waterfall, Periodogram, Transmit)
2. -Log Contact- and -PTT- (push to talk) (Waterfall, Periodogram, Transmit)
3. -Navigation and ID- (Waterfall, Periodogram, Transmit)
4. -Band Selection- -AF Gain- and -RF Gain- (Waterfall, Periodogram, Settings)

The Frequency selection widgets operate as old-time push-the-digit
rotary counters. MB1 on the top of the digit increments it. MB1 on the
bottom of the digit decrements it. As digits roll-over or roll-under
they change the digit to their left.

-Log Contact- will copy the information from the from/to grids and the from/to callsign boxes, the current time, and the transmit and receive frequencies to a log entry that can be seen in the Log Editor panel. 

\subsubsection Navigation and Logging

\subsubsection Band Selection

The band selection box appears on the two RX oriented panels and the settings panel. Surprisingly, this has proved useful during the experimental phase while I was testing out hardware functions. It is particularly useful when attempting to find frequency corrections for transverters that may not have ultra-accurate oscillators. (See @ref transvertersGUI)

\subsubsection Gain Settings

The RF and AF gain settings are present on the two RX oriented panels
and the settings panel. Early use showed that the most frequent reason
for switching panels was to bump the volume or adjust the RF gain. 

\subsection thePanels Panels

Information on the state of the radio, its input signals, and controls
is presented in a set of panels. SoDaRadio avoids popups where possible, so
changing a control setting often involves changing a panel. This felt 
better than pop-up boxes, so there you are. 

Normally we'd navigate from panel to panel with tabs. In fact, that's how
the FlatSoDa GUI works. But the tabs take up valuable display space and offer
almost no information. There are two ways to bring up a panel: 
1. Click `<MB2>` to bring up a radio-button box. Mouse over your selection and click `<MB1>`. Note that the `<MB2>` button does *not* work when pressed over a black
region. There's a reason for this. Wish I knew what it was.
2. Use the keyboard. Each panel gets its own key `<ALT>W` for the
waterfall, `<ALT>P` for the periodogram, `<ALT>S` for the settings, `<ALT>L`
for the log, `<ALT>C` for the button box in option 1, `<ALT>V` for the VU
meter, and `<ALT>H` to remind you of what the other buttons do.

I used `<ALT>` rather than just the bare letter. That seemed like a good idea
at the time. Let me know if you have strong feelings otherwise. 

So, what are these panels? 

\subsubsection  waterfallPanel Waterfall

The Waterfall Panel \image html waterfall_note.png width=700px


The horizontal axis is labeled with the receive frequency. The vertical axis shows the evolution of the input signals with time. The color axis shows the power spectral density at each frequency. 

The Signal Strength is represented by a color map ranging from indigo (comparatively weak) to red (comparatively strong).  Note that these are on a more-or-less arbitrary absolute scale. The differences (in dB) are what matter. (Just like the Periodogram.) So the color map "slides" over the possible ranges of densities. You can set the reference level by fiddling the spinbox under "Floor (dB)" until you see a slightly brighter than indigo background. 

You'll notice that signals are not pinpoints. That's how signal processing goes. Some of the spread will be due to the modulation. This is especially clear if you find a "clicky" CW signal - there will be little spikes to the left and right of the blotch. 

I generally keep the range at 50 dB from indigo to red. But if you've really got a yen to increase the contrast, the selector gives you that option. 

I have found the span control to be quite useful. Wide span (up to 200kHz) helps when you've no idea where the other station is. Narrow the span when you want that close up look, perhaps to find a really weak station who's frequency is known (or at least suspected). 

The average window and update rate do what they say. Want the display to scroll faster? Wiggle the Update Rate. Want to smooth things out a bit? Fiddle the Average window. 

The "Center RX Freq" button shifts the frequency axis so that the current RX frequency is shown in the center. 

The waterfall is hot: you can poke a point on the waterfall and the RX
frequency will change to the corresponding frequency. This allows
rapid tuning. Poke with MB1 to change the frequency. Poking with MB3
will change the frequency *and* move the new frequency to the center
of the display.

Activate the waterfall with `<ALT>W`.  Exiting the Transmit panel by
hitting the "PTT" button will take you to the waterfall panel.

If the waterfall plot is blank (dark) then the floor is set *too
high*. Poke the down button or use the scroll wheel (or even type!) to
reduce the floor setting until you see something useful.

\subsubsection  periodogramPanel Periodogram

The Periodogram Panel presents a plot of power-spectral-density across the band of interest. 

The "Center RX Freq" "Average Window" and "Span" controls work just as they do
in the waterfall display. The "RefLevel (dB) sets the relative signal strength
at the top of the plot. At times, 

As with the waterfall plot, the periodogram area is hot. Poke with MB1 to tune. Poke with MB3 to tune and shift the frequency axis. 

The Periodogram (Spectrum Power) Panel \image html
periodogram_note.png width=700px

\subsubsection  transmitPanel Transmit

\subsubsection  settingsPanel Settings

\subsubsection  configBandsPanel Config Bands

\subsubsection  editLogPanel Edit Log

\subsubsection statusPanel  Status

\subsubsection theVuMeter The Vu Meter

The periodogram is a good tuning aid for dish positioning and such. However
some might prefer a simpler presentation. 

An (audio) signal strength meter \image html
vumeter_note.png width=700px 

The Vu meter doesn't have NIST traceable accuracy, but it might help. Hit <alt>V to bring it up and <alt>V to make it go away. The Vu meter is the only "pop-up" window in SoDaRadio. I couldn't figure out how to integrate it into the major views without making other things less useful. 

\section radioSettings Settings

\subsection settingsMode  The Mode and Filter Selectors
\subsection settingsAudio Audio Input and Output Device Selection
\subsection settingsGains The AF and RF Gain Controls
\subsection settingsTX  Transmitter Settings
\subsection settingsStationID QSO Settings
\subsection settingsRecord 
\subsection settingsBandsAnts

The band selector is what it is. The options that appear there are 
defined by the [band configuration settings.](@ref definingBands)

\subsection settingsConfiguration The Configuration Options



\subsection settingsRefOsc The Reference Oscillator

Some SDR hardware provides an input connection for a high-accuracy reference oscillator. It is surprising how far off that nice 1ppm crystal in your HammerCrafters SuperHam 3000 SDR can be. By the time you get to 2304 MHz you can be off by 2 kHz. Some radios deal with that by using really really good crystals. Or a GPS disciplined oscillator. If you have a radio that does this, and a reference oscillator, hook 'em up and tick the "External Reference" box.  

(As a note to Ettus users, due to a quirk in the design of the N200 series, the main oscillator is a TCXO that is disciplined by a separate oscillator that is disciplined by the external reference oscillator.  For reasons I don't understand, the main oscillator becomes more stable (and often more accurate) when the external reference is selected *even when there is nothing connected to its input*. As best I can tell, the input oscillator is a much better device than the main unit. Go figure. YMMV.)

\subsection settingsMisc Miscellaneous Settings


\subsection helpBrowser The Help Browser

If you are reading this, then you've probably already found the help browser. 
You got here from the Settings panel by poking the "Help" button. 

\section definingBands  Defining Bands

\subsection simpleDefBands Simple definitions

\subsection transvertersGUI Transverters 

\subsection theTheme The Theme
Is dark. Yes. We'll try it in the sun and see how things work. 

\section configFiles Configuration Files

\section firstConfig Creating Your First Configuration File

\section modelNames Model Names

Some product families (in particular, the Ettus radios) include
devices with multiple tuning ranges. My USRP B200, for instance, has a
nominal tuning range of 70 MHz to 6 GHz. My USRP N200, however, can
tune from about 1MHz to 6 GHz.  I don't need a transverter for the
N200, but the B200 does.  So SoDaRadio recognizes two names for the
USRP product line.  USRPB200 will select a configuration file called
"USRPB200_SoDaRadioQT.conf" where you would want to define band
entries with apprpriate transverters.

Selecting "USRP" will open a configuration file
"USRP_SoDaRadioQT.conf" that probably has no entries for transverters
at frequencies below 5 GHz.

\section theLog The Log



