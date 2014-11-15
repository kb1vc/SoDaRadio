///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 21 2009)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "SoDaRadio_GUI.h"

///////////////////////////////////////////////////////////////////////////
using namespace SoDaRadio_GUI;

SoDaRadioFrame::SoDaRadioFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menubar1 = new wxMenuBar( 0 );
	FileMenu = new wxMenu();
	wxMenuItem* OpenConfig;
	OpenConfig = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Load Configuration...") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( OpenConfig );
	
	wxMenuItem* SaveConfig;
	SaveConfig = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Save Configuration") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( SaveConfig );
	
	wxMenuItem* SaveConfigAs;
	SaveConfigAs = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Save Configuration As...") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( SaveConfigAs );
	
	wxMenuItem* OpenLogfile;
	OpenLogfile = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Open Logfile...") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( OpenLogfile );
	
	wxMenuItem* Quit;
	Quit = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Quit") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( Quit );
	
	m_menubar1->Append( FileMenu, wxT("&File") );
	
	ConfigMenu = new wxMenu();
	wxMenuItem* SetMyCallsign;
	SetMyCallsign = new wxMenuItem( ConfigMenu, wxID_ANY, wxString( wxT("Set Callsign...") ) , wxEmptyString, wxITEM_NORMAL );
	ConfigMenu->Append( SetMyCallsign );
	
	wxMenuItem* SetMyGrid;
	SetMyGrid = new wxMenuItem( ConfigMenu, wxID_ANY, wxString( wxT("Set Current Grid...") ) , wxEmptyString, wxITEM_NORMAL );
	ConfigMenu->Append( SetMyGrid );
	
	wxMenuItem* GPSOn;
	GPSOn = new wxMenuItem( ConfigMenu, wxID_ANY, wxString( wxT("GPS On") ) , wxEmptyString, wxITEM_CHECK );
	ConfigMenu->Append( GPSOn );
	
	wxMenuItem* m_configureBand;
	m_configureBand = new wxMenuItem( ConfigMenu, wxID_ANY, wxString( wxT("Configure Band") ) , wxEmptyString, wxITEM_NORMAL );
	ConfigMenu->Append( m_configureBand );
	
	wxMenuItem* m_configSpect;
	m_configSpect = new wxMenuItem( ConfigMenu, wxID_ANY, wxString( wxT("Configure Spectrum Display") ) , wxEmptyString, wxITEM_NORMAL );
	ConfigMenu->Append( m_configSpect );
	
	m_menubar1->Append( ConfigMenu, wxT("Configure") );
	
	m_QSOMenu = new wxMenu();
	wxMenuItem* m_SetToCall;
	m_SetToCall = new wxMenuItem( m_QSOMenu, ID_GOTOCALL, wxString( wxT("Set To Call") ) + wxT('\t') + wxT("Ctrl+C"), wxEmptyString, wxITEM_NORMAL );
	m_QSOMenu->Append( m_SetToCall );
	
	wxMenuItem* m_SetToGrid;
	m_SetToGrid = new wxMenuItem( m_QSOMenu, ID_GOTOGRID, wxString( wxT("Set To Grid") ) + wxT('\t') + wxT("Ctrl+G"), wxEmptyString, wxITEM_NORMAL );
	m_QSOMenu->Append( m_SetToGrid );
	
	wxMenuItem* m_GoToComment;
	m_GoToComment = new wxMenuItem( m_QSOMenu, ID_GOTOLOG, wxString( wxT("Fill In Comment") ) + wxT('\t') + wxT("Ctrl+L"), wxEmptyString, wxITEM_NORMAL );
	m_QSOMenu->Append( m_GoToComment );
	
	wxMenuItem* m_GoToMessage;
	m_GoToMessage = new wxMenuItem( m_QSOMenu, ID_GOTOMSG, wxString( wxT("Enter CW Text") ) + wxT('\t') + wxT("Ctrl+X"), wxEmptyString, wxITEM_NORMAL );
	m_QSOMenu->Append( m_GoToMessage );
	
	wxMenuItem* m_ToggleTX;
	m_ToggleTX = new wxMenuItem( m_QSOMenu, ID_TOGGLETX, wxString( wxT("Toggle TX Switch") ) + wxT('\t') + wxT("Ctrl+T"), wxEmptyString, wxITEM_NORMAL );
	m_QSOMenu->Append( m_ToggleTX );
	
	m_menubar1->Append( m_QSOMenu, wxT("QSO Actions") );
	
	m_bandSelect = new wxMenu();
	m_menubar1->Append( m_bandSelect, wxT("Select Band") );
	
	HelpMenu = new wxMenu();
	wxMenuItem* Aboutitem;
	Aboutitem = new wxMenuItem( HelpMenu, wxID_ANY, wxString( wxT("About") ) , wxEmptyString, wxITEM_NORMAL );
	HelpMenu->Append( Aboutitem );
	
	wxMenuItem* UserGuideitem;
	UserGuideitem = new wxMenuItem( HelpMenu, wxID_ANY, wxString( wxT("User's Guide") ) , wxEmptyString, wxITEM_NORMAL );
	HelpMenu->Append( UserGuideitem );
	
	m_menubar1->Append( HelpMenu, wxT("Help") );
	
	this->SetMenuBar( m_menubar1 );
	
	m_ClueBar = this->CreateStatusBar( 3, wxST_SIZEGRIP, wxID_ANY );
	wxBoxSizer* TopSizer;
	TopSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxHORIZONTAL );
	
	SpectrumDisplay = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	WaterFallPanel = new wxPanel( SpectrumDisplay, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	SpectrumDisplay->AddPage( WaterFallPanel, wxT("Waterfall"), false );
	FFTPanel = new wxPanel( SpectrumDisplay, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	SpectrumDisplay->AddPage( FFTPanel, wxT("Periodogram"), true );
	
	bSizer51->Add( SpectrumDisplay, 1, wxEXPAND, 5 );
	
	TopSizer->Add( bSizer51, 5, wxEXPAND, 5 );
	
	wxBoxSizer* ControlSizer;
	ControlSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* RadCtrlSizer;
	RadCtrlSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer35;
	sbSizer35 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mode") ), wxVERTICAL );
	
	wxString m_ModeBoxChoices[] = { wxT("CW_U"), wxT("USB"), wxT("CW_L"), wxT("LSB"), wxT("AM"), wxT("WBFM"), wxT("NBFM") };
	int m_ModeBoxNChoices = sizeof( m_ModeBoxChoices ) / sizeof( wxString );
	m_ModeBox = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ModeBoxNChoices, m_ModeBoxChoices, 0 );
	m_ModeBox->SetSelection( 0 );
	sbSizer35->Add( m_ModeBox, 0, wxALL, 5 );
	
	RadCtrlSizer->Add( sbSizer35, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* AFBWSizer;
	AFBWSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("AF Bandwidth") ), wxVERTICAL );
	
	wxString m_AFBWChoiceChoices[] = { wxT("100 Hz"), wxT("500 Hz"), wxT("2000 Hz"), wxT("6000 Hz") };
	int m_AFBWChoiceNChoices = sizeof( m_AFBWChoiceChoices ) / sizeof( wxString );
	m_AFBWChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_AFBWChoiceNChoices, m_AFBWChoiceChoices, 0 );
	m_AFBWChoice->SetSelection( 2 );
	AFBWSizer->Add( m_AFBWChoice, 0, wxALL, 5 );
	
	RadCtrlSizer->Add( AFBWSizer, 1, 0, 5 );
	
	wxStaticBoxSizer* AFGSizer;
	AFGSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("AF Gain") ), wxVERTICAL );
	
	m_AFGain = new wxSlider( this, wxID_ANY, 15, 1, 50, wxDefaultPosition, wxSize( -1,-1 ), wxSL_HORIZONTAL );
	m_AFGain->SetMinSize( wxSize( 100,-1 ) );
	
	AFGSizer->Add( m_AFGain, 0, wxALL, 5 );
	
	RadCtrlSizer->Add( AFGSizer, 1, 0, 5 );
	
	wxStaticBoxSizer* RFGSizer;
	RFGSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("RF Gain") ), wxVERTICAL );
	
	m_RFGain = new wxSlider( this, wxID_ANY, 40, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_RFGain->SetMinSize( wxSize( 100,-1 ) );
	
	RFGSizer->Add( m_RFGain, 0, wxALL, 5 );
	
	RadCtrlSizer->Add( RFGSizer, 1, 0, 5 );
	
	ControlSizer->Add( RadCtrlSizer, 1, 0, 5 );
	
	
	ControlSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbTxFreq;
	sbTxFreq = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TX Frequency") ), wxVERTICAL );
	
	sbTxFreq->SetMinSize( wxSize( 230,-1 ) ); 
	m_TXFreqText = new wxStaticText( this, wxID_ANY, wxT("10,368.100 000"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_TXFreqText->Wrap( -1 );
	m_TXFreqText->SetFont( wxFont( 20, 70, 90, 90, false, wxEmptyString ) );
	
	sbTxFreq->Add( m_TXFreqText, 0, wxALL, 5 );
	
	ControlSizer->Add( sbTxFreq, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxVERTICAL );
	
	m_TXRXLocked = new wxCheckBox( this, wxID_ANY, wxT("TX=RX Lock"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TXRXLocked->SetValue(true); 
	bSizer71->Add( m_TXRXLocked, 0, wxALL, 5 );
	
	m_SpecTrack = new wxCheckBox( this, wxID_ANY, wxT("Spect. Track"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SpecTrack->SetValue(true); 
	bSizer71->Add( m_SpecTrack, 0, wxALL, 5 );
	
	ControlSizer->Add( bSizer71, 0, 0, 5 );
	
	wxStaticBoxSizer* sbRxFreq;
	sbRxFreq = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("RX Frequency") ), wxVERTICAL );
	
	sbRxFreq->SetMinSize( wxSize( 230,-1 ) ); 
	m_RXFreqText = new wxStaticText( this, wxID_ANY, wxT("10,368.100 000"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_RXFreqText->Wrap( -1 );
	m_RXFreqText->SetFont( wxFont( 20, 70, 90, 90, false, wxEmptyString ) );
	
	sbRxFreq->Add( m_RXFreqText, 0, wxALL, 5 );
	
	ControlSizer->Add( sbRxFreq, 0, wxEXPAND, 5 );
	
	wxBoxSizer* sTuneSpec;
	sTuneSpec = new wxBoxSizer( wxHORIZONTAL );
	
	Tune = new wxButton( this, wxID_ANY, wxT("Tune"), wxDefaultPosition, wxDefaultSize, 0 );
	Tune->SetFont( wxFont( 20, 70, 90, 92, false, wxEmptyString ) );
	
	sTuneSpec->Add( Tune, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* sSpec;
	sSpec = new wxBoxSizer( wxVERTICAL );
	
	m_RX2CF = new wxButton( this, wxID_ANY, wxT("RX -> CFreq"), wxDefaultPosition, wxDefaultSize, 0 );
	sSpec->Add( m_RX2CF, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_ConfigSpectrum = new wxButton( this, wxID_ANY, wxT("Display..."), wxDefaultPosition, wxDefaultSize, 0 );
	sSpec->Add( m_ConfigSpectrum, 0, wxALL, 5 );
	
	sTuneSpec->Add( sSpec, 1, wxEXPAND, 5 );
	
	ControlSizer->Add( sTuneSpec, 1, wxEXPAND, 5 );
	
	TopSizer->Add( ControlSizer, 1, 0, 5 );
	
	wxBoxSizer* QSOSizer;
	QSOSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	wxGridBagSizer* gbSizer1;
	gbSizer1 = new wxGridBagSizer( 0, 0 );
	gbSizer1->SetFlexibleDirection( wxBOTH );
	gbSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer15;
	sbSizer15 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Transmit CW Text") ), wxVERTICAL );
	
	m_CWTextOutbound = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CHARWRAP|wxTE_MULTILINE|wxTE_READONLY|wxVSCROLL );
	m_CWTextOutbound->Enable( false );
	m_CWTextOutbound->SetMinSize( wxSize( 800,-1 ) );
	
	sbSizer15->Add( m_CWTextOutbound, 2, wxALL|wxEXPAND, 5 );
	
	m_CWTextEntry = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_CWTextEntry->SetMinSize( wxSize( -1,40 ) );
	
	sbSizer15->Add( m_CWTextEntry, 0, wxALL|wxEXPAND, 5 );
	
	gbSizer1->Add( sbSizer15, wxGBPosition( 0, 2 ), wxGBSpan( 3, 3 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer21;
	sbSizer21 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("From Grid") ), wxVERTICAL );
	
	m_MyGrid = new wxStaticText( this, wxID_ANY, wxT("FM19ag"), wxDefaultPosition, wxDefaultSize, 0 );
	m_MyGrid->Wrap( -1 );
	sbSizer21->Add( m_MyGrid, 0, wxALL, 5 );
	
	gbSizer1->Add( sbSizer21, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer31;
	sbSizer31 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("To Grid") ), wxVERTICAL );
	
	m_ToGrid = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	sbSizer31->Add( m_ToGrid, 0, wxALL, 5 );
	
	gbSizer1->Add( sbSizer31, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("To Call") ), wxVERTICAL );
	
	m_ToCall = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	sbSizer41->Add( m_ToCall, 0, wxALL, 5 );
	
	gbSizer1->Add( sbSizer41, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer62;
	sbSizer62 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Bearing") ), wxVERTICAL );
	
	m_BearingText = new wxStaticText( this, wxID_ANY, wxT("000 T 015M"), wxDefaultPosition, wxDefaultSize, 0 );
	m_BearingText->Wrap( -1 );
	sbSizer62->Add( m_BearingText, 0, wxALL, 5 );
	
	gbSizer1->Add( sbSizer62, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer621;
	sbSizer621 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Rev. Bearing") ), wxVERTICAL );
	
	m_RevBearingText = new wxStaticText( this, wxID_ANY, wxT("000 T 015M"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RevBearingText->Wrap( -1 );
	sbSizer621->Add( m_RevBearingText, 0, wxALL, 5 );
	
	gbSizer1->Add( sbSizer621, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer611;
	sbSizer611 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Range") ), wxVERTICAL );
	
	m_RangeText = new wxStaticText( this, wxID_ANY, wxT("35 km"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RangeText->Wrap( -1 );
	sbSizer611->Add( m_RangeText, 0, wxALL, 5 );
	
	gbSizer1->Add( sbSizer611, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* CWCtlSizer;
	CWCtlSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("CW Control") ), wxHORIZONTAL );
	
	m_CWsendEx = new wxButton( this, wxID_ANY, wxT("Exchange"), wxDefaultPosition, wxDefaultSize, 0 );
	CWCtlSizer->Add( m_CWsendEx, 0, wxALL, 5 );
	
	m_CWsendInfo = new wxButton( this, wxID_ANY, wxT("My Info"), wxDefaultPosition, wxSize( 75,-1 ), 0 );
	CWCtlSizer->Add( m_CWsendInfo, 0, wxALL, 5 );
	
	m_CWsendCall = new wxButton( this, wxID_ANY, wxT("My Call"), wxDefaultPosition, wxSize( 75,-1 ), 0 );
	CWCtlSizer->Add( m_CWsendCall, 0, wxALL, 5 );
	
	m_CWsendGrid = new wxButton( this, wxID_ANY, wxT("My Grid"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	CWCtlSizer->Add( m_CWsendGrid, 0, wxALL, 5 );
	
	m_CWsendQSL = new wxButton( this, wxID_ANY, wxT("QSL"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	CWCtlSizer->Add( m_CWsendQSL, 0, wxALL, 5 );
	
	m_CWsendBK = new wxButton( this, wxID_ANY, wxT("BK"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_CWsendBK->SetForegroundColour( wxColour( 255, 255, 255 ) );
	m_CWsendBK->SetBackgroundColour( wxColour( 19, 43, 223 ) );
	
	CWCtlSizer->Add( m_CWsendBK, 0, wxALL, 5 );
	
	m_CWsend73 = new wxButton( this, wxID_ANY, wxT("73"), wxDefaultPosition, wxSize( 45,-1 ), 0 );
	m_CWsend73->SetForegroundColour( wxColour( 255, 255, 255 ) );
	m_CWsend73->SetBackgroundColour( wxColour( 19, 43, 223 ) );
	
	CWCtlSizer->Add( m_CWsend73, 0, wxALL, 5 );
	
	m_CWsendV = new wxButton( this, wxID_ANY, wxT("V"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	CWCtlSizer->Add( m_CWsendV, 0, wxALL, 5 );
	
	m_CWsendCarrier = new wxButton( this, wxID_ANY, wxT("Carrier"), wxDefaultPosition, wxSize( 75,-1 ), 0 );
	CWCtlSizer->Add( m_CWsendCarrier, 0, wxALL, 5 );
	
	bSizer91->Add( CWCtlSizer, 9, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer37;
	sbSizer37 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Repeat") ), wxVERTICAL );
	
	m_RepeatCount = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), wxSP_ARROW_KEYS, 1, 30, 1 );
	sbSizer37->Add( m_RepeatCount, 0, wxALL, 5 );
	
	bSizer91->Add( sbSizer37, 1, 0, 5 );
	
	gbSizer1->Add( bSizer91, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );
	
	m_clrCWbutton = new wxButton( this, wxID_ANY, wxT("Clear CW Buffer"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_clrCWbutton, 0, wxALL, 5 );
	
	gbSizer1->Add( bSizer14, wxGBPosition( 3, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL, 5 );
	
	m_button10 = new wxButton( this, wxID_ANY, wxT("Log Contact"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer1->Add( m_button10, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_EditLogButton = new wxButton( this, wxID_ANY, wxT("Edit Log"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer1->Add( m_EditLogButton, wxGBPosition( 4, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Log Comment") ), wxVERTICAL );
	
	m_LogCommentBox = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_LogCommentBox->SetMinSize( wxSize( 500,-1 ) );
	
	sbSizer16->Add( m_LogCommentBox, 0, wxALL|wxEXPAND, 5 );
	
	gbSizer1->Add( sbSizer16, wxGBPosition( 4, 1 ), wxGBSpan( 1, 2 ), wxEXPAND, 5 );
	
	wxBoxSizer* CtrlPopSizer;
	CtrlPopSizer = new wxBoxSizer( wxVERTICAL );
	
	CtrlPopup = new wxButton( this, wxID_ANY, wxT("Set Power, Speed, Sidetone"), wxDefaultPosition, wxDefaultSize, 0 );
	CtrlPopSizer->Add( CtrlPopup, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	gbSizer1->Add( CtrlPopSizer, wxGBPosition( 3, 0 ), wxGBSpan( 1, 2 ), wxEXPAND, 5 );
	
	bSizer7->Add( gbSizer1, 0, wxEXPAND, 5 );
	
	QSOSizer->Add( bSizer7, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer29;
	sbSizer29 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TX PTT") ), wxVERTICAL );
	
	m_PTT = new wxButton( this, wxID_ANY, wxT(" TX\nOFF"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PTT->SetFont( wxFont( 30, 70, 90, 92, false, wxEmptyString ) );
	m_PTT->SetForegroundColour( wxColour( 240, 240, 240 ) );
	m_PTT->SetBackgroundColour( wxColour( 19, 43, 223 ) );
	
	sbSizer29->Add( m_PTT, 0, wxALIGN_CENTER|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer53;
	bSizer53 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer34;
	sbSizer34 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TIME (UTC)") ), wxVERTICAL );
	
	m_UTC = new wxStaticText( this, wxID_ANY, wxT("00::00:00"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_UTC->Wrap( -1 );
	sbSizer34->Add( m_UTC, 0, wxALL, 5 );
	
	bSizer54->Add( sbSizer34, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer321;
	sbSizer321 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Grid") ), wxHORIZONTAL );
	
	m_GPSGrid = new wxStaticText( this, wxID_ANY, wxT("XX99xx"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_GPSGrid->Wrap( -1 );
	sbSizer321->Add( m_GPSGrid, 0, wxALL, 5 );
	
	bSizer54->Add( sbSizer321, 1, wxEXPAND, 5 );
	
	bSizer53->Add( bSizer54, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer33;
	sbSizer33 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LAT") ), wxHORIZONTAL );
	
	m_GPSLat = new wxStaticText( this, wxID_ANY, wxT("00.00"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_GPSLat->Wrap( -1 );
	sbSizer33->Add( m_GPSLat, 0, wxALL, 5 );
	
	bSizer55->Add( sbSizer33, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer331;
	sbSizer331 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("LON") ), wxHORIZONTAL );
	
	m_GPSLon = new wxStaticText( this, wxID_ANY, wxT("00.00"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_GPSLon->Wrap( -1 );
	sbSizer331->Add( m_GPSLon, 0, wxALL, 5 );
	
	bSizer55->Add( sbSizer331, 1, wxEXPAND, 5 );
	
	bSizer53->Add( bSizer55, 1, wxEXPAND, 5 );
	
	sbSizer29->Add( bSizer53, 1, wxEXPAND, 5 );
	
	QSOSizer->Add( sbSizer29, 0, 0, 5 );
	
	TopSizer->Add( QSOSizer, 5, wxEXPAND, 5 );
	
	this->SetSizer( TopSizer );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( SoDaRadioFrame::OnClose ) );
	this->Connect( OpenConfig->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnOpenConfig ) );
	this->Connect( SaveConfig->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSaveConfig ) );
	this->Connect( SaveConfigAs->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSaveConfigAs ) );
	this->Connect( OpenLogfile->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnOpenLogfile ) );
	this->Connect( Quit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQuit ) );
	this->Connect( SetMyCallsign->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSetFromCall ) );
	this->Connect( SetMyGrid->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSetFromGrid ) );
	this->Connect( GPSOn->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnGPSOnSel ) );
	this->Connect( m_configureBand->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnConfigBand ) );
	this->Connect( m_configSpect->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnMenuConfigSpect ) );
	this->Connect( m_SetToCall->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Connect( m_SetToGrid->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Connect( m_GoToComment->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Connect( m_GoToMessage->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Connect( m_ToggleTX->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Connect( Aboutitem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnAbout ) );
	this->Connect( UserGuideitem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnUserGuide ) );
	SpectrumDisplay->Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SoDaRadioFrame::OnOpenSpectConfig ), NULL, this );
	SpectrumDisplay->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( SoDaRadioFrame::OnSelectPage ), NULL, this );
	SpectrumDisplay->Connect( wxEVT_PAINT, wxPaintEventHandler( SoDaRadioFrame::OnPaintWaterfall ), NULL, this );
	SpectrumDisplay->Connect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( SoDaRadioFrame::OnWFallFreqSel ), NULL, this );
	WaterFallPanel->Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SoDaRadioFrame::OnOpenSpectConfig ), NULL, this );
	FFTPanel->Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SoDaRadioFrame::OnOpenSpectConfig ), NULL, this );
	FFTPanel->Connect( wxEVT_PAINT, wxPaintEventHandler( SoDaRadioFrame::OnPaintPeriodogram ), NULL, this );
	FFTPanel->Connect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( SoDaRadioFrame::OnPeriodogramFreqSel ), NULL, this );
	m_ModeBox->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnModeChoice ), NULL, this );
	m_AFBWChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnAFBWChoice ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_TXRXLocked->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnTXRXLock ), NULL, this );
	Tune->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnTunePopup ), NULL, this );
	m_RX2CF->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnPerRxToCentFreq ), NULL, this );
	m_ConfigSpectrum->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnMenuConfigSpect ), NULL, this );
	m_CWTextEntry->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnSendText ), NULL, this );
	m_ToGrid->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( SoDaRadioFrame::OnNewToGrid ), NULL, this );
	m_ToGrid->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnNewToGridEnter ), NULL, this );
	m_ToCall->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( SoDaRadioFrame::OnNewToCall ), NULL, this );
	m_ToCall->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnNewToCall ), NULL, this );
	m_CWsendEx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendInfo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendCall->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendGrid->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendQSL->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendBK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsend73->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendV->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendCarrier->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_clrCWbutton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnClrCWBuffer ), NULL, this );
	m_button10->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnLogContact ), NULL, this );
	m_EditLogButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnEditLog ), NULL, this );
	m_LogCommentBox->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnSaveComment ), NULL, this );
	CtrlPopup->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCtrlPopup ), NULL, this );
	m_PTT->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnTXOnOff ), NULL, this );
}

SoDaRadioFrame::~SoDaRadioFrame()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( SoDaRadioFrame::OnClose ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnOpenConfig ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSaveConfig ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSaveConfigAs ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnOpenLogfile ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQuit ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSetFromCall ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnSetFromGrid ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnGPSOnSel ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnConfigBand ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnMenuConfigSpect ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnQSOMenuSet ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnAbout ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnUserGuide ) );
	SpectrumDisplay->Disconnect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SoDaRadioFrame::OnOpenSpectConfig ), NULL, this );
	SpectrumDisplay->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( SoDaRadioFrame::OnSelectPage ), NULL, this );
	SpectrumDisplay->Disconnect( wxEVT_PAINT, wxPaintEventHandler( SoDaRadioFrame::OnPaintWaterfall ), NULL, this );
	SpectrumDisplay->Disconnect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( SoDaRadioFrame::OnWFallFreqSel ), NULL, this );
	WaterFallPanel->Disconnect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SoDaRadioFrame::OnOpenSpectConfig ), NULL, this );
	FFTPanel->Disconnect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SoDaRadioFrame::OnOpenSpectConfig ), NULL, this );
	FFTPanel->Disconnect( wxEVT_PAINT, wxPaintEventHandler( SoDaRadioFrame::OnPaintPeriodogram ), NULL, this );
	FFTPanel->Disconnect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( SoDaRadioFrame::OnPeriodogramFreqSel ), NULL, this );
	m_ModeBox->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnModeChoice ), NULL, this );
	m_AFBWChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaRadioFrame::OnAFBWChoice ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_AFGain->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( SoDaRadioFrame::OnAFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_RFGain->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( SoDaRadioFrame::OnRFGainScroll ), NULL, this );
	m_TXRXLocked->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnTXRXLock ), NULL, this );
	Tune->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnTunePopup ), NULL, this );
	m_RX2CF->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnPerRxToCentFreq ), NULL, this );
	m_ConfigSpectrum->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnMenuConfigSpect ), NULL, this );
	m_CWTextEntry->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnSendText ), NULL, this );
	m_ToGrid->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( SoDaRadioFrame::OnNewToGrid ), NULL, this );
	m_ToGrid->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnNewToGridEnter ), NULL, this );
	m_ToCall->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( SoDaRadioFrame::OnNewToCall ), NULL, this );
	m_ToCall->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnNewToCall ), NULL, this );
	m_CWsendEx->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendInfo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendCall->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendGrid->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendQSL->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendBK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsend73->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendV->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_CWsendCarrier->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCWControl ), NULL, this );
	m_clrCWbutton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnClrCWBuffer ), NULL, this );
	m_button10->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnLogContact ), NULL, this );
	m_EditLogButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnEditLog ), NULL, this );
	m_LogCommentBox->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaRadioFrame::OnSaveComment ), NULL, this );
	CtrlPopup->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnCtrlPopup ), NULL, this );
	m_PTT->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SoDaRadioFrame::OnTXOnOff ), NULL, this );
}

m_LogDialog::m_LogDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* logSizer;
	logSizer = new wxBoxSizer( wxVERTICAL );
	
	m_LogGrid = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxSize( -1,250 ), 0 );
	
	// Grid
	m_LogGrid->CreateGrid( 1, 10 );
	m_LogGrid->EnableEditing( true );
	m_LogGrid->EnableGridLines( true );
	m_LogGrid->EnableDragGridSize( false );
	m_LogGrid->SetMargins( 0, 0 );
	
	// Columns
	m_LogGrid->SetColSize( 0, 167 );
	m_LogGrid->SetColSize( 1, 80 );
	m_LogGrid->SetColSize( 2, 80 );
	m_LogGrid->SetColSize( 3, 80 );
	m_LogGrid->SetColSize( 4, 80 );
	m_LogGrid->SetColSize( 5, 61 );
	m_LogGrid->SetColSize( 6, 97 );
	m_LogGrid->SetColSize( 7, 97 );
	m_LogGrid->SetColSize( 8, 68 );
	m_LogGrid->SetColSize( 9, 200 );
	m_LogGrid->EnableDragColMove( false );
	m_LogGrid->EnableDragColSize( true );
	m_LogGrid->SetColLabelSize( 30 );
	m_LogGrid->SetColLabelValue( 0, wxT("Date/Time (UTC)") );
	m_LogGrid->SetColLabelValue( 1, wxT("From Call") );
	m_LogGrid->SetColLabelValue( 2, wxT("From Grid") );
	m_LogGrid->SetColLabelValue( 3, wxT("To Call") );
	m_LogGrid->SetColLabelValue( 4, wxT("To Grid") );
	m_LogGrid->SetColLabelValue( 5, wxT("Mode") );
	m_LogGrid->SetColLabelValue( 6, wxT("TX Freq") );
	m_LogGrid->SetColLabelValue( 7, wxT("RX Freq") );
	m_LogGrid->SetColLabelValue( 8, wxT("Distance") );
	m_LogGrid->SetColLabelValue( 9, wxT("Comment") );
	m_LogGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_LogGrid->EnableDragRowSize( true );
	m_LogGrid->SetRowLabelSize( 80 );
	m_LogGrid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_LogGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	logSizer->Add( m_LogGrid, 9, wxALL|wxEXPAND, 5 );
	
	m_LogOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	logSizer->Add( m_LogOK, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	this->SetSizer( logSizer );
	this->Layout();
	
	// Connect Events
	m_LogGrid->Connect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( m_LogDialog::OnLogCellChange ), NULL, this );
	m_LogOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_LogDialog::OnLogOK ), NULL, this );
}

m_LogDialog::~m_LogDialog()
{
	// Disconnect Events
	m_LogGrid->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( m_LogDialog::OnLogCellChange ), NULL, this );
	m_LogOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_LogDialog::OnLogOK ), NULL, this );
}

m_AboutDialog::m_AboutDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxVERTICAL );
	
	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("SoDaLogo_Big.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer57->Add( m_bitmap1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_staticText15 = new wxStaticText( this, wxID_ANY, wxT("SoDa Radio\nThe 'SoD' stands for Software Defined.\n The 'a' doesn't stand for anything.\n\n(Copyright 2013 kb1vc)\n  All rights reserved.\n\n  Redistribution and use in source and binary forms, with or without\n  modification, are permitted provided that the following conditions are\n  met:\n\n  Redistributions of source code must retain the above copyright\n  notice, this list of conditions and the following disclaimer.\n  Redistributions in binary form must reproduce the above copyright\n  notice, this list of conditions and the following disclaimer in\n  the documentation and/or other materials provided with the\n  distribution.\n\n  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n  \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText15->Wrap( -1 );
	bSizer57->Add( m_staticText15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_GUIVersion = new wxStaticText( this, wxID_ANY, wxT("GUI Version: "), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_GUIVersion->Wrap( -1 );
	bSizer57->Add( m_GUIVersion, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_SDRVersion = new wxStaticText( this, wxID_ANY, wxT("SDR Version"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_SDRVersion->Wrap( -1 );
	bSizer57->Add( m_SDRVersion, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_staticline5 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer57->Add( m_staticline5, 0, wxEXPAND | wxALL, 5 );
	
	m_AboutOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer57->Add( m_AboutOK, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	this->SetSizer( bSizer57 );
	this->Layout();
	
	// Connect Events
	m_AboutOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_AboutDialog::OnAboutOK ), NULL, this );
}

m_AboutDialog::~m_AboutDialog()
{
	// Disconnect Events
	m_AboutOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_AboutDialog::OnAboutOK ), NULL, this );
}

m_QuitDialogConfig::m_QuitDialogConfig( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText10 = new wxStaticText( this, wxID_ANY, wxT("You've made some changes to the configuration.\nShould we save the changes?"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText10->Wrap( -1 );
	bSizer37->Add( m_staticText10, 0, wxALIGN_CENTER|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxHORIZONTAL );
	
	m_QuitNSave = new wxButton( this, wxID_ANY, wxT("Save Configuration"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer38->Add( m_QuitNSave, 0, wxALL, 5 );
	
	
	bSizer38->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_QuitNoSave = new wxButton( this, wxID_ANY, wxT("Ignore Changes"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer38->Add( m_QuitNoSave, 0, wxALL, 5 );
	
	bSizer37->Add( bSizer38, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSizer37 );
	this->Layout();
	bSizer37->Fit( this );
	
	// Connect Events
	m_QuitNSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_QuitDialogConfig::SaveConfigChanges ), NULL, this );
	m_QuitNoSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_QuitDialogConfig::OnIgnoreConfigChanges ), NULL, this );
}

m_QuitDialogConfig::~m_QuitDialogConfig()
{
	// Disconnect Events
	m_QuitNSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_QuitDialogConfig::SaveConfigChanges ), NULL, this );
	m_QuitNoSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_QuitDialogConfig::OnIgnoreConfigChanges ), NULL, this );
}

m_SpectConfigDialog::m_SpectConfigDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer32;
	sbSizer32 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Band Spread") ), wxVERTICAL );
	
	sbSizer32->SetMinSize( wxSize( 120,-1 ) ); 
	wxString m_BandSpreadChoiceChoices[] = { wxT("25 kHz"), wxT("50 kHz"), wxT("100 kHz"), wxT("200 kHz"), wxT("500 kHz") };
	int m_BandSpreadChoiceNChoices = sizeof( m_BandSpreadChoiceChoices ) / sizeof( wxString );
	m_BandSpreadChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_BandSpreadChoiceNChoices, m_BandSpreadChoiceChoices, 0 );
	m_BandSpreadChoice->SetSelection( 1 );
	sbSizer32->Add( m_BandSpreadChoice, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	bSizer81->Add( sbSizer32, 0, wxEXPAND|wxTOP, 5 );
	
	
	bSizer81->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer18;
	sbSizer18 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Center Frequency") ), wxVERTICAL );
	
	sbSizer18->SetMinSize( wxSize( 140,-1 ) ); 
	m_cFreqSpin = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10368000, 10368400, 10368100 );
	sbSizer18->Add( m_cFreqSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer81->Add( sbSizer18, 0, wxEXPAND|wxTOP, 5 );
	
	bSizer37->Add( bSizer81, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbPgSettingsSizer;
	sbPgSettingsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Periodogram") ), wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer371;
	sbSizer371 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Window Length") ), wxVERTICAL );
	
	m_PeriodogramWindowSel = new wxSlider( this, wxID_ANY, 20, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_PeriodogramWindowSel->SetMinSize( wxSize( 100,50 ) );
	
	sbSizer371->Add( m_PeriodogramWindowSel, 0, wxALL, 5 );
	
	sbPgSettingsSizer->Add( sbSizer371, 1, wxEXPAND|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer19;
	sbSizer19 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Y Rng (dB/box)") ), wxVERTICAL );
	
	wxString m_dBScaleChoices[] = { wxT("1dB"), wxT("5dB"), wxT("10dB"), wxT("20dB") };
	int m_dBScaleNChoices = sizeof( m_dBScaleChoices ) / sizeof( wxString );
	m_dBScale = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_dBScaleNChoices, m_dBScaleChoices, 0 );
	m_dBScale->SetSelection( 0 );
	sbSizer19->Add( m_dBScale, 0, wxALL, 5 );
	
	sbPgSettingsSizer->Add( sbSizer19, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer191;
	sbSizer191 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Y RefLevel (dB)") ), wxVERTICAL );
	
	m_RefLevel = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -150, 40, 0 );
	sbSizer191->Add( m_RefLevel, 0, wxALL, 5 );
	
	sbPgSettingsSizer->Add( sbSizer191, 1, wxEXPAND|wxTOP, 5 );
	
	bSizer37->Add( sbPgSettingsSizer, 0, wxBOTTOM|wxEXPAND|wxTOP, 10 );
	
	wxStaticBoxSizer* sbWfSettingsSizer;
	sbWfSettingsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Waterfall") ), wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer36;
	sbSizer36 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Window Length") ), wxVERTICAL );
	
	m_WaterfallWindowSel = new wxSlider( this, wxID_ANY, 4, 1, 25, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_WaterfallWindowSel->SetMinSize( wxSize( 100,-1 ) );
	
	sbSizer36->Add( m_WaterfallWindowSel, 0, wxALL, 5 );
	
	sbWfSettingsSizer->Add( sbSizer36, 1, wxEXPAND|wxTOP, 5 );
	
	
	sbWfSettingsSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer351;
	sbSizer351 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Scroll Speed") ), wxVERTICAL );
	
	m_WaterfallScrollSpeed = new wxSlider( this, wxID_ANY, 1, 4, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_WaterfallScrollSpeed->SetMinSize( wxSize( 100,-1 ) );
	
	sbSizer351->Add( m_WaterfallScrollSpeed, 0, wxALL, 5 );
	
	sbWfSettingsSizer->Add( sbSizer351, 1, wxEXPAND|wxTOP, 5 );
	
	bSizer37->Add( sbWfSettingsSizer, 0, wxBOTTOM|wxEXPAND, 15 );
	
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer38->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_OK = new wxButton( this, wxID_ANY, wxT("Done"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer38->Add( m_OK, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxEXPAND, 15 );
	
	
	bSizer38->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bSizer37->Add( bSizer38, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	this->SetSizer( bSizer37 );
	this->Layout();
	bSizer37->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( m_SpectConfigDialog::OnDone ) );
	m_BandSpreadChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( m_SpectConfigDialog::OnPerBandSpread ), NULL, this );
	m_cFreqSpin->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( m_SpectConfigDialog::OnPerCFreqStep ), NULL, this );
	m_PeriodogramWindowSel->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_SpectConfigDialog::OnWindowLenUpdate ), NULL, this );
	m_dBScale->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( m_SpectConfigDialog::OnPerYScaleChoice ), NULL, this );
	m_RefLevel->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( m_SpectConfigDialog::OnPerRefLevel ), NULL, this );
	m_WaterfallWindowSel->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_SpectConfigDialog::OnWindowLenUpdate ), NULL, this );
	m_WaterfallScrollSpeed->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_SpectConfigDialog::OnScrollSpeedUpdate ), NULL, this );
	m_OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_SpectConfigDialog::OnDone ), NULL, this );
}

m_SpectConfigDialog::~m_SpectConfigDialog()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( m_SpectConfigDialog::OnDone ) );
	m_BandSpreadChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( m_SpectConfigDialog::OnPerBandSpread ), NULL, this );
	m_cFreqSpin->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( m_SpectConfigDialog::OnPerCFreqStep ), NULL, this );
	m_PeriodogramWindowSel->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_SpectConfigDialog::OnWindowLenUpdate ), NULL, this );
	m_dBScale->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( m_SpectConfigDialog::OnPerYScaleChoice ), NULL, this );
	m_RefLevel->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( m_SpectConfigDialog::OnPerRefLevel ), NULL, this );
	m_WaterfallWindowSel->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_SpectConfigDialog::OnWindowLenUpdate ), NULL, this );
	m_WaterfallScrollSpeed->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_SpectConfigDialog::OnScrollSpeedUpdate ), NULL, this );
	m_OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_SpectConfigDialog::OnDone ), NULL, this );
}

m_TuningDialog::m_TuningDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* FreqSizer;
	FreqSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* txFreqSizer;
	txFreqSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TX Frequency") ), wxVERTICAL );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* fdig10G;
	fdig10G = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT10 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10G->Add( m_DigitUpT10, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT10 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT10->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig10G->Add( digitTextT10, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT10 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10G->Add( m_DigitDownT10, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig10G, 1, 0, 2 );
	
	wxBoxSizer* fdig1G;
	fdig1G = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT9 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1G->Add( m_DigitUpT9, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT9 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT9->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1G->Add( digitTextT9, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT9 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1G->Add( m_DigitDownT9, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig1G, 1, wxEXPAND, 5 );
	
	m_staticText8 = new wxStaticText( this, wxID_ANY, wxT(","), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	m_staticText8->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	bSizer21->Add( m_staticText8, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* fdig100M;
	fdig100M = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT8 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100M->Add( m_DigitUpT8, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT8 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT8->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig100M->Add( digitTextT8, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT8 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100M->Add( m_DigitDownT8, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig100M, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig10M;
	fdig10M = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT7 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10M->Add( m_DigitUpT7, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT7 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT7->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig10M->Add( digitTextT7, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT7 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10M->Add( m_DigitDownT7, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig10M, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig1M;
	fdig1M = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT6 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1M->Add( m_DigitUpT6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT6 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT6->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1M->Add( digitTextT6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT6 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1M->Add( m_DigitDownT6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig1M, 1, wxEXPAND, 5 );
	
	m_staticText811 = new wxStaticText( this, wxID_ANY, wxT(","), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText811->Wrap( -1 );
	m_staticText811->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	bSizer21->Add( m_staticText811, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* fdig100K;
	fdig100K = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT5 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100K->Add( m_DigitUpT5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT5 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT5->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig100K->Add( digitTextT5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT5 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100K->Add( m_DigitDownT5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig100K, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig10K;
	fdig10K = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT4 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10K->Add( m_DigitUpT4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT4 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT4->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig10K->Add( digitTextT4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT4 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10K->Add( m_DigitDownT4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig10K, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig1K;
	fdig1K = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT3 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1K->Add( m_DigitUpT3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT3 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT3->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1K->Add( digitTextT3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT3 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1K->Add( m_DigitDownT3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig1K, 1, wxEXPAND, 5 );
	
	m_staticText81 = new wxStaticText( this, wxID_ANY, wxT(","), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	bSizer21->Add( m_staticText81, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* fdig100;
	fdig100 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT2 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100->Add( m_DigitUpT2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT2 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT2->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig100->Add( digitTextT2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT2 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100->Add( m_DigitDownT2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig100, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig10;
	fdig10 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT1 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10->Add( m_DigitUpT1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT1 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT1->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig10->Add( digitTextT1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT1 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10->Add( m_DigitDownT1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig10, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig1;
	fdig1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpT0 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1->Add( m_DigitUpT0, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextT0 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextT0->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1->Add( digitTextT0, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownT0 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1->Add( m_DigitDownT0, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer21->Add( fdig1, 1, wxEXPAND, 5 );
	
	bSizer20->Add( bSizer21, 1, 0, 5 );
	
	b_tx_2_rx = new wxButton( this, wxID_ANY, wxT("TX->RX"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer20->Add( b_tx_2_rx, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	b_last_tx = new wxButton( this, wxID_ANY, wxT("Last TX"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer20->Add( b_last_tx, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	txFreqSizer->Add( bSizer20, 1, wxEXPAND, 5 );
	
	FreqSizer->Add( txFreqSizer, 0, 0, 5 );
	
	wxStaticBoxSizer* rxFreqSizer;
	rxFreqSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("RX Frequency") ), wxVERTICAL );
	
	wxBoxSizer* bSizer201;
	bSizer201 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* fdig10G1;
	fdig10G1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR10 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10G1->Add( m_DigitUpR10, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR10 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR10->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig10G1->Add( digitTextR10, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR10 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10G1->Add( m_DigitDownR10, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig10G1, 1, 0, 2 );
	
	wxBoxSizer* fdig1G1;
	fdig1G1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR9 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1G1->Add( m_DigitUpR9, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR9 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR9->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1G1->Add( digitTextR9, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR9 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1G1->Add( m_DigitDownR9, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig1G1, 1, wxEXPAND, 5 );
	
	m_staticText82 = new wxStaticText( this, wxID_ANY, wxT(","), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText82->Wrap( -1 );
	m_staticText82->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	bSizer211->Add( m_staticText82, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* fdig100M1;
	fdig100M1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR8 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100M1->Add( m_DigitUpR8, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR8 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR8->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig100M1->Add( digitTextR8, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR8 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100M1->Add( m_DigitDownR8, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig100M1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig10M1;
	fdig10M1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR7 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10M1->Add( m_DigitUpR7, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR7 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR7->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig10M1->Add( digitTextR7, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR7 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10M1->Add( m_DigitDownR7, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig10M1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig1M1;
	fdig1M1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR6 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1M1->Add( m_DigitUpR6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR6 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR6->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1M1->Add( digitTextR6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR6 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1M1->Add( m_DigitDownR6, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig1M1, 1, wxEXPAND, 5 );
	
	m_staticText8111 = new wxStaticText( this, wxID_ANY, wxT(","), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8111->Wrap( -1 );
	m_staticText8111->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	bSizer211->Add( m_staticText8111, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* fdig100K1;
	fdig100K1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR5 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100K1->Add( m_DigitUpR5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR5 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR5->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig100K1->Add( digitTextR5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR5 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig100K1->Add( m_DigitDownR5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig100K1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig10K1;
	fdig10K1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR4 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10K1->Add( m_DigitUpR4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR4 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR4->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig10K1->Add( digitTextR4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR4 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig10K1->Add( m_DigitDownR4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig10K1, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig1K1;
	fdig1K1 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR3 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1K1->Add( m_DigitUpR3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR3 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR3->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1K1->Add( digitTextR3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR3 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1K1->Add( m_DigitDownR3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig1K1, 1, wxEXPAND, 5 );
	
	m_staticText812 = new wxStaticText( this, wxID_ANY, wxT(","), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText812->Wrap( -1 );
	m_staticText812->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	bSizer211->Add( m_staticText812, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* fdig1001;
	fdig1001 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR2 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1001->Add( m_DigitUpR2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR2 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR2->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig1001->Add( digitTextR2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR2 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig1001->Add( m_DigitDownR2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig1001, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig101;
	fdig101 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR1 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig101->Add( m_DigitUpR1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR1 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR1->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig101->Add( digitTextR1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR1 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig101->Add( m_DigitDownR1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig101, 1, wxEXPAND, 5 );
	
	wxBoxSizer* fdig11;
	fdig11 = new wxBoxSizer( wxVERTICAL );
	
	m_DigitUpR0 = new wxBitmapButton( this, DIG_UP, wxBitmap( wxT("uparrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig11->Add( m_DigitUpR0, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	digitTextR0 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 30,30 ), wxTE_READONLY );
	digitTextR0->SetFont( wxFont( 24, 70, 90, 90, false, wxEmptyString ) );
	
	fdig11->Add( digitTextR0, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	m_DigitDownR0 = new wxBitmapButton( this, wxID_ANY, wxBitmap( wxT("downarrow.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( 30,18 ), wxBU_AUTODRAW );
	fdig11->Add( m_DigitDownR0, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 2 );
	
	bSizer211->Add( fdig11, 1, wxEXPAND, 5 );
	
	bSizer201->Add( bSizer211, 0, 0, 5 );
	
	b_tx_2_rx1 = new wxButton( this, wxID_ANY, wxT("RX->TX"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer201->Add( b_tx_2_rx1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	b_last_tx1 = new wxButton( this, wxID_ANY, wxT("Last RX"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer201->Add( b_last_tx1, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	rxFreqSizer->Add( bSizer201, 1, wxEXPAND, 5 );
	
	FreqSizer->Add( rxFreqSizer, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ExtRefEn = new wxCheckBox( this, wxID_ANY, wxT("Ext Ref Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ExtRefEn->SetValue(true); 
	bSizer42->Add( m_ExtRefEn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer42->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	m_RefStatus = new wxStaticText( this, wxID_ANY, wxT("Ref Locked"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RefStatus->Wrap( -1 );
	bSizer42->Add( m_RefStatus, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer42->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );
	
	m_RXLOStatus1 = new wxStaticText( this, wxID_ANY, wxT("RX LO Locked"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RXLOStatus1->Wrap( -1 );
	bSizer42->Add( m_RXLOStatus1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer42->Add( m_staticline3, 0, wxEXPAND | wxALL, 5 );
	
	m_TXLOStatus = new wxStaticText( this, wxID_ANY, wxT("TX LO Locked"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TXLOStatus->Wrap( -1 );
	bSizer42->Add( m_TXLOStatus, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer42->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );
	
	m_ReSyncUWLO = new wxButton( this, wxID_ANY, wxT("Transverter LO Cal"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer42->Add( m_ReSyncUWLO, 0, wxALL, 5 );
	
	FreqSizer->Add( bSizer42, 1, wxEXPAND, 5 );
	
	m_TuneDone = new wxButton( this, wxID_ANY, wxT("Done"), wxDefaultPosition, wxDefaultSize, 0 );
	FreqSizer->Add( m_TuneDone, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	this->SetSizer( FreqSizer );
	this->Layout();
	FreqSizer->Fit( this );
	
	// Connect Events
	m_DigitUpT10->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT10->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT9->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT9->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT8->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT8->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT7->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT7->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT5->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT5->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT4->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT4->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT0->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT0->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	b_tx_2_rx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnCopyTXtoRX ), NULL, this );
	b_last_tx->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnLastTX ), NULL, this );
	m_DigitUpR10->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR10->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR9->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR9->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR8->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR8->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR7->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR7->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR6->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR5->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR5->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR4->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR4->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR3->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR2->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR0->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR0->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	b_tx_2_rx1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnCopyRXtoTX ), NULL, this );
	b_last_tx1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnLastRX ), NULL, this );
	m_ExtRefEn->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( m_TuningDialog::OnExtRefEna ), NULL, this );
	m_ReSyncUWLO->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnTransvLOCal ), NULL, this );
	m_TuneDone->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnTuningDone ), NULL, this );
}

m_TuningDialog::~m_TuningDialog()
{
	// Disconnect Events
	m_DigitUpT10->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT10->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT9->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT9->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT8->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT8->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT7->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT7->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT5->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT5->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT4->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT4->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpT0->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownT0->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	b_tx_2_rx->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnCopyTXtoRX ), NULL, this );
	b_last_tx->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnLastTX ), NULL, this );
	m_DigitUpR10->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR10->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR9->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR9->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR8->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR8->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR7->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR7->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR6->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR5->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR5->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR4->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR4->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR3->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR2->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	m_DigitUpR0->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitUp ), NULL, this );
	m_DigitDownR0->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnDigitDown ), NULL, this );
	b_tx_2_rx1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnCopyRXtoTX ), NULL, this );
	b_last_tx1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnLastRX ), NULL, this );
	m_ExtRefEn->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( m_TuningDialog::OnExtRefEna ), NULL, this );
	m_ReSyncUWLO->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnTransvLOCal ), NULL, this );
	m_TuneDone->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_TuningDialog::OnTuningDone ), NULL, this );
}

m_ControlsDialog::m_ControlsDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* controlSizer;
	controlSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* PowerSizer;
	PowerSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TX Power") ), wxVERTICAL );
	
	m_TXPower = new wxSlider( this, wxID_ANY, 7, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_TXPower->SetMinSize( wxSize( 100,-1 ) );
	
	PowerSizer->Add( m_TXPower, 0, wxALL, 5 );
	
	bSizer58->Add( PowerSizer, 0, wxEXPAND, 5 );
	
	bSizer57->Add( bSizer58, 1, wxEXPAND, 5 );
	
	wxBoxSizer* KeyerSizer;
	KeyerSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* SideToneVolSizer;
	SideToneVolSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("SideTone Vol") ), wxVERTICAL );
	
	m_STGain = new wxSlider( this, wxID_ANY, 20, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_STGain->SetMinSize( wxSize( 100,-1 ) );
	
	SideToneVolSizer->Add( m_STGain, 0, wxALL, 5 );
	
	KeyerSizer->Add( SideToneVolSizer, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* CWSpeedSizer;
	CWSpeedSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("CW WPM") ), wxVERTICAL );
	
	m_CWSpeed = new wxSlider( this, wxID_ANY, 10, 5, 50, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_CWSpeed->SetMinSize( wxSize( 100,-1 ) );
	
	CWSpeedSizer->Add( m_CWSpeed, 0, wxALL, 5 );
	
	KeyerSizer->Add( CWSpeedSizer, 0, wxEXPAND, 5 );
	
	bSizer57->Add( KeyerSizer, 1, wxEXPAND, 5 );
	
	controlSizer->Add( bSizer57, 1, wxEXPAND, 5 );
	
	m_CtrlDone = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	controlSizer->Add( m_CtrlDone, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	this->SetSizer( controlSizer );
	this->Layout();
	controlSizer->Fit( this );
	
	// Connect Events
	m_TXPower->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CtrlDone->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_ControlsDialog::OnCtrlDone ), NULL, this );
}

m_ControlsDialog::~m_ControlsDialog()
{
	// Disconnect Events
	m_TXPower->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_TXPower->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( m_ControlsDialog::OnTXPower ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_STGain->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( m_ControlsDialog::OnSTGainScroll ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CWSpeed->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( m_ControlsDialog::OnCWSpeed ), NULL, this );
	m_CtrlDone->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_ControlsDialog::OnCtrlDone ), NULL, this );
}

m_NewConfigDialog::m_NewConfigDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxVERTICAL );
	
	
	bSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText25 = new wxStaticText( this, wxID_ANY, wxT("SoDaRadio could not find a configuration file.\nSelect \"OK\" to create a configuration file in\n${HOME}/.SoDaRadio/SoDa.soda_cfg\""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText25->Wrap( -1 );
	bSizer58->Add( m_staticText25, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_StatusInfo = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_StatusInfo->Wrap( -1 );
	bSizer58->Add( m_StatusInfo, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer59;
	bSizer59 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer59->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_CreateConfigDefault = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer59->Add( m_CreateConfigDefault, 0, wxALL, 5 );
	
	
	bSizer59->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_NoThanksCreateConfigDefault = new wxButton( this, wxID_ANY, wxT("No Thank You"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer59->Add( m_NoThanksCreateConfigDefault, 0, wxALL, 5 );
	
	
	bSizer59->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bSizer58->Add( bSizer59, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer58 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_CreateConfigDefault->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_NewConfigDialog::OnCreateConfigDefault ), NULL, this );
	m_NoThanksCreateConfigDefault->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_NewConfigDialog::OnDismissCreateConfigDefault ), NULL, this );
}

m_NewConfigDialog::~m_NewConfigDialog()
{
	// Disconnect Events
	m_CreateConfigDefault->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_NewConfigDialog::OnCreateConfigDefault ), NULL, this );
	m_NoThanksCreateConfigDefault->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_NewConfigDialog::OnDismissCreateConfigDefault ), NULL, this );
}

m_BandConfigDialog::m_BandConfigDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer60;
	bSizer60 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText36 = new wxStaticText( this, wxID_ANY, wxT("Band Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	bSizer72->Add( m_staticText36, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_BandChoiceBoxChoices[] = { wxT("Create New Band") };
	int m_BandChoiceBoxNChoices = sizeof( m_BandChoiceBoxChoices ) / sizeof( wxString );
	m_BandChoiceBox = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_BandChoiceBoxNChoices, m_BandChoiceBoxChoices, 0 );
	m_BandChoiceBox->SetSelection( 0 );
	m_BandChoiceBox->SetMinSize( wxSize( 200,-1 ) );
	
	bSizer72->Add( m_BandChoiceBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_BandName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_BandName->SetMaxLength( 16 ); 
	m_BandName->SetMinSize( wxSize( 200,-1 ) );
	
	bSizer72->Add( m_BandName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	bSizer60->Add( bSizer72, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer67;
	bSizer67 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* AntFreqRange;
	AntFreqRange = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer62;
	bSizer62 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText27 = new wxStaticText( this, wxID_ANY, wxT("RX Antenna"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_staticText27->Wrap( -1 );
	bSizer62->Add( m_staticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_RXAntChoiceChoices[] = { wxT("TX/RX"), wxT("RX2") };
	int m_RXAntChoiceNChoices = sizeof( m_RXAntChoiceChoices ) / sizeof( wxString );
	m_RXAntChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_RXAntChoiceNChoices, m_RXAntChoiceChoices, 0 );
	m_RXAntChoice->SetSelection( 0 );
	bSizer62->Add( m_RXAntChoice, 0, wxALL, 5 );
	
	AntFreqRange->Add( bSizer62, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer70;
	bSizer70 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText31 = new wxStaticText( this, wxID_ANY, wxT("Lower Band Edge"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText31->Wrap( -1 );
	bSizer70->Add( m_staticText31, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_low_edge = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer70->Add( m_low_edge, 0, wxALL, 5 );
	
	m_staticText34 = new wxStaticText( this, wxID_ANY, wxT("MHz"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText34->Wrap( -1 );
	bSizer70->Add( m_staticText34, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	AntFreqRange->Add( bSizer70, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText32 = new wxStaticText( this, wxID_ANY, wxT("Upper Band Edge"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText32->Wrap( -1 );
	bSizer71->Add( m_staticText32, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_high_edge = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer71->Add( m_high_edge, 0, wxALL, 5 );
	
	m_staticText33 = new wxStaticText( this, wxID_ANY, wxT("MHz"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText33->Wrap( -1 );
	bSizer71->Add( m_staticText33, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	AntFreqRange->Add( bSizer71, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer76;
	bSizer76 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText39 = new wxStaticText( this, wxID_ANY, wxT("Default Modulation Mode"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText39->Wrap( -1 );
	bSizer76->Add( m_staticText39, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_ModChoiceChoices[] = { wxT("CW_U"), wxT("CW_L"), wxT("USB"), wxT("LSB"), wxT("AM"), wxT("NBFM"), wxT("WBFM") };
	int m_ModChoiceNChoices = sizeof( m_ModChoiceChoices ) / sizeof( wxString );
	m_ModChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ModChoiceNChoices, m_ModChoiceChoices, 0 );
	m_ModChoice->SetSelection( 0 );
	bSizer76->Add( m_ModChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	AntFreqRange->Add( bSizer76, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxHORIZONTAL );
	
	m_TXEna = new wxCheckBox( this, wxID_ANY, wxT("Enable Transmit"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	bSizer77->Add( m_TXEna, 0, wxALL, 5 );
	
	AntFreqRange->Add( bSizer77, 1, wxEXPAND, 5 );
	
	bSizer67->Add( AntFreqRange, 1, wxEXPAND, 5 );
	
	wxBoxSizer* TransverterSetup;
	TransverterSetup = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer78;
	bSizer78 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText41 = new wxStaticText( this, wxID_ANY, wxT("Band ID Number"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	bSizer78->Add( m_staticText41, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_BandID = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 0 );
	bSizer78->Add( m_BandID, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	TransverterSetup->Add( bSizer78, 1, wxEXPAND, 5 );
	
	m_TransverterMode = new wxCheckBox( this, wxID_ANY, wxT("Transverter Mode"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	TransverterSetup->Add( m_TransverterMode, 0, wxALL, 5 );
	
	m_LOGenMode = new wxCheckBox( this, wxID_ANY, wxT("LO from TX2 Port"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	m_LOGenMode->Enable( false );
	
	TransverterSetup->Add( m_LOGenMode, 0, wxALL, 5 );
	
	wxString m_InjectionSelChoices[] = { wxT("Low Side"), wxT("High Side") };
	int m_InjectionSelNChoices = sizeof( m_InjectionSelChoices ) / sizeof( wxString );
	m_InjectionSel = new wxRadioBox( this, wxID_ANY, wxT("Injection"), wxDefaultPosition, wxDefaultSize, m_InjectionSelNChoices, m_InjectionSelChoices, 1, wxRA_SPECIFY_ROWS );
	m_InjectionSel->SetSelection( 0 );
	m_InjectionSel->Enable( false );
	
	TransverterSetup->Add( m_InjectionSel, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxHORIZONTAL );
	
	m_TransFreqLabel = new wxStaticText( this, wxID_ANY, wxT("Transverter LO Frequency"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TransFreqLabel->Wrap( -1 );
	m_TransFreqLabel->Enable( false );
	
	bSizer63->Add( m_TransFreqLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_TransFreqEntry = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TransFreqEntry->Enable( false );
	
	bSizer63->Add( m_TransFreqEntry, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_TransFreqLabel2 = new wxStaticText( this, wxID_ANY, wxT("MHz"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TransFreqLabel2->Wrap( -1 );
	m_TransFreqLabel2->Enable( false );
	
	bSizer63->Add( m_TransFreqLabel2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	TransverterSetup->Add( bSizer63, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer631;
	bSizer631 = new wxBoxSizer( wxHORIZONTAL );
	
	m_TransMultLabel = new wxStaticText( this, wxID_ANY, wxT("Transverter Multiplier"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TransMultLabel->Wrap( -1 );
	m_TransMultLabel->Enable( false );
	
	bSizer631->Add( m_TransMultLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_TransMultEntry = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TransMultEntry->Enable( false );
	
	bSizer631->Add( m_TransMultEntry, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	TransverterSetup->Add( bSizer631, 1, wxEXPAND, 5 );
	
	bSizer67->Add( TransverterSetup, 1, wxEXPAND, 5 );
	
	bSizer60->Add( bSizer67, 3, wxEXPAND, 5 );
	
	m_staticline6 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer60->Add( m_staticline6, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer61->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_BandCancel = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer61->Add( m_BandCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer61->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_BandOK = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer61->Add( m_BandOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer61->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bSizer60->Add( bSizer61, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer60 );
	this->Layout();
	bSizer60->Fit( this );
	
	// Connect Events
	m_BandChoiceBox->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( m_BandConfigDialog::OnConfigChoice ), NULL, this );
	m_TransverterMode->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnTransverterModeSel ), NULL, this );
	m_LOGenMode->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnTransverterModeSel ), NULL, this );
	m_BandCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnBandCancel ), NULL, this );
	m_BandOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnBandOK ), NULL, this );
}

m_BandConfigDialog::~m_BandConfigDialog()
{
	// Disconnect Events
	m_BandChoiceBox->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( m_BandConfigDialog::OnConfigChoice ), NULL, this );
	m_TransverterMode->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnTransverterModeSel ), NULL, this );
	m_LOGenMode->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnTransverterModeSel ), NULL, this );
	m_BandCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnBandCancel ), NULL, this );
	m_BandOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_BandConfigDialog::OnBandOK ), NULL, this );
}

m_BandConfigProblem::m_BandConfigProblem( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer79;
	bSizer79 = new wxBoxSizer( wxVERTICAL );
	
	m_BrokenBandForm = new wxStaticText( this, wxID_ANY, wxT("There was a problem in defining the new band.\nPlease correct the form and try again."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_BrokenBandForm->Wrap( -1 );
	bSizer79->Add( m_BrokenBandForm, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_BandConfigReason = new wxStaticText( this, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_BandConfigReason->Wrap( -1 );
	bSizer79->Add( m_BandConfigReason, 2, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_button38 = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer79->Add( m_button38, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	this->SetSizer( bSizer79 );
	this->Layout();
	
	// Connect Events
	m_button38->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_BandConfigProblem::OnBandErrorOK ), NULL, this );
}

m_BandConfigProblem::~m_BandConfigProblem()
{
	// Disconnect Events
	m_button38->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( m_BandConfigProblem::OnBandErrorOK ), NULL, this );
}
