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

like right here: 

\section fred separate section

\section gui_basics The GUI Presentation 

some text here


\subsection thePanels Panels

\subsubsection  waterfallPanel Waterfall

The Waterfall Panel \image html waterfall_note.png width=800px

\subsubsection  periodogramPanel Periodogram

The Periodogram (Spectrum Power) Panel \image html
periodogram_note.png width=800px

\subsubsection  transmitPanel Transmit

\subsubsection  settingsPanel Settings

\subsubsection  configBandsPanel Config Bands

\subsubsection  editLogPanel Edit Log

\subsubsection statusPanel  Status

\subsubsection theVuMeter The Vu Meter

\section radioSettings Settings

\section definingBands  Defining Bands

\subsection simpleDefBands Simple definitions

\subsection transvertersGUI Transverters 


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



