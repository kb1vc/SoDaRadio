///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul  5 2013)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "SoDaSNA_GUI.h"

///////////////////////////////////////////////////////////////////////////
using namespace SoDaSNA_GUI;

SoDaSNAFrame::SoDaSNAFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
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
	
	wxMenuItem* Calibrate;
	Calibrate = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Calibrate...") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( Calibrate );
	
	wxMenuItem* Quit;
	Quit = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Quit") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( Quit );
	
	m_menubar1->Append( FileMenu, wxT("&File") ); 
	
	HelpMenu = new wxMenu();
	wxMenuItem* Aboutitem;
	Aboutitem = new wxMenuItem( HelpMenu, wxID_ANY, wxString( wxT("About") ) , wxEmptyString, wxITEM_NORMAL );
	HelpMenu->Append( Aboutitem );
	
	wxMenuItem* UserGuideitem;
	UserGuideitem = new wxMenuItem( HelpMenu, wxID_ANY, wxString( wxT("User's Guide") ) , wxEmptyString, wxITEM_NORMAL );
	HelpMenu->Append( UserGuideitem );
	
	m_menubar1->Append( HelpMenu, wxT("Help") ); 
	
	this->SetMenuBar( m_menubar1 );
	
	wxBoxSizer* TopSizer;
	TopSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sGenSizer;
	sGenSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxStaticBoxSizer* XYSizer;
	XYSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("mag(S21) dB") ), wxVERTICAL );
	
	m_DisplayPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( 700,-1 ), wxTAB_TRAVERSAL );
	XYSizer->Add( m_DisplayPanel, 1, wxEXPAND | wxALL, 5 );
	
	sGenSizer->Add( XYSizer, 1, wxEXPAND, 5 );
	
	wxGridBagSizer* gbSizer2;
	gbSizer2 = new wxGridBagSizer( 0, 0 );
	gbSizer2->SetFlexibleDirection( wxBOTH );
	gbSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxStaticBoxSizer* sSweepSpeed;
	sSweepSpeed = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Sweep Speed") ), wxVERTICAL );
	
	sSweepSpeed->SetMinSize( wxSize( 100,-1 ) ); 
	wxString m_SweepRateChoices[] = { wxT("1 S"), wxT("10 S") };
	int m_SweepRateNChoices = sizeof( m_SweepRateChoices ) / sizeof( wxString );
	m_SweepRate = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_SweepRateNChoices, m_SweepRateChoices, 0 );
	m_SweepRate->SetSelection( 3 );
	sSweepSpeed->Add( m_SweepRate, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	gbSizer2->Add( sSweepSpeed, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), 0, 5 );
	
	wxStaticBoxSizer* sFreqCentSizer;
	sFreqCentSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Center Frequency") ), wxHORIZONTAL );
	
	m_CenterFreqBox = new wxTextCtrl( this, wxID_ANY, wxT("150.0"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB );
	m_CenterFreqBox->SetMinSize( wxSize( 100,-1 ) );
	
	sFreqCentSizer->Add( m_CenterFreqBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_CenterUnitsChoices[] = { wxT("kHz"), wxT("MHz"), wxT("GHz") };
	int m_CenterUnitsNChoices = sizeof( m_CenterUnitsChoices ) / sizeof( wxString );
	m_CenterUnits = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_CenterUnitsNChoices, m_CenterUnitsChoices, 0 );
	m_CenterUnits->SetSelection( 1 );
	sFreqCentSizer->Add( m_CenterUnits, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	gbSizer2->Add( sFreqCentSizer, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), 0, 5 );
	
	wxStaticBoxSizer* sFreqSpanSizer;
	sFreqSpanSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Frequency Span") ), wxHORIZONTAL );
	
	wxString m_SpanMagChoices[] = { wxT("1"), wxT("2"), wxT("5"), wxT("10"), wxT("20"), wxT("50"), wxT("100"), wxT("200"), wxT("500") };
	int m_SpanMagNChoices = sizeof( m_SpanMagChoices ) / sizeof( wxString );
	m_SpanMag = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_SpanMagNChoices, m_SpanMagChoices, 0 );
	m_SpanMag->SetSelection( 0 );
	sFreqSpanSizer->Add( m_SpanMag, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_SpanUnitsChoices[] = { wxT("kHz"), wxT("MHz"), wxT("GHz") };
	int m_SpanUnitsNChoices = sizeof( m_SpanUnitsChoices ) / sizeof( wxString );
	m_SpanUnits = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_SpanUnitsNChoices, m_SpanUnitsChoices, 0 );
	m_SpanUnits->SetSelection( 1 );
	sFreqSpanSizer->Add( m_SpanUnits, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	gbSizer2->Add( sFreqSpanSizer, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbOutputPowerSizerA;
	sbOutputPowerSizerA = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Relative Output Power (dB)") ), wxHORIZONTAL );
	
	m_OutPowerSliderA = new wxSlider( this, wxID_ANY, 10, -30, 0, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_BOTH|wxSL_HORIZONTAL|wxSL_LABELS );
	m_OutPowerSliderA->SetMinSize( wxSize( 200,-1 ) );
	
	sbOutputPowerSizerA->Add( m_OutPowerSliderA, 0, wxALL, 5 );
	
	gbSizer2->Add( sbOutputPowerSizerA, wxGBPosition( 1, 1 ), wxGBSpan( 1, 2 ), wxEXPAND, 5 );
	
	wxBoxSizer* sRFOutA;
	sRFOutA = new wxBoxSizer( wxVERTICAL );
	
	gbSizer2->Add( sRFOutA, wxGBPosition( 0, 5 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	wxStaticBoxSizer* sPlotControl;
	sPlotControl = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Reference Level") ), wxHORIZONTAL );
	
	m_RefLevel = new wxSpinCtrl( this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -70, 30, 0 );
	sPlotControl->Add( m_RefLevel, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	gbSizer2->Add( sPlotControl, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), 0, 5 );
	
	wxStaticBoxSizer* sVrtRes;
	sVrtRes = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Vertical Resolution") ), wxHORIZONTAL );
	
	wxString m_SpanMag1Choices[] = { wxT("1"), wxT("2"), wxT("5"), wxT("10") };
	int m_SpanMag1NChoices = sizeof( m_SpanMag1Choices ) / sizeof( wxString );
	m_SpanMag1 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_SpanMag1NChoices, m_SpanMag1Choices, 0 );
	m_SpanMag1->SetSelection( 3 );
	sVrtRes->Add( m_SpanMag1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("dB / box"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	sVrtRes->Add( m_staticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	gbSizer2->Add( sVrtRes, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxString m_SweepControlChoices[] = { wxT("Off"), wxT("Single"), wxT("Continuous") };
	int m_SweepControlNChoices = sizeof( m_SweepControlChoices ) / sizeof( wxString );
	m_SweepControl = new wxRadioBox( this, wxID_ANY, wxT("Sweep Control"), wxDefaultPosition, wxDefaultSize, m_SweepControlNChoices, m_SweepControlChoices, 1, wxRA_SPECIFY_COLS );
	m_SweepControl->SetSelection( 0 );
	gbSizer2->Add( m_SweepControl, wxGBPosition( 0, 4 ), wxGBSpan( 2, 1 ), wxALL, 5 );
	
	sGenSizer->Add( gbSizer2, 0, wxEXPAND, 5 );
	
	TopSizer->Add( sGenSizer, 7, wxEXPAND, 5 );
	
	this->SetSizer( TopSizer );
	this->Layout();
	
	// Connect Events
	this->Connect( OpenConfig->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnOpenConfig ) );
	this->Connect( SaveConfig->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSaveConfig ) );
	this->Connect( SaveConfigAs->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSaveConfigAs ) );
	this->Connect( Calibrate->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnCalibrate ) );
	this->Connect( Quit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnQuit ) );
	this->Connect( Aboutitem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnAbout ) );
	this->Connect( UserGuideitem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnUserGuide ) );
	m_SweepRate->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSweepSpeed ), NULL, this );
	m_CenterFreqBox->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( SoDaSNAFrame::OnFreqEnter ), NULL, this );
	m_CenterFreqBox->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaSNAFrame::OnFreqEnter ), NULL, this );
	m_CenterUnits->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_SpanMag->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_SpanUnits->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_SpanMag1->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_SweepControl->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSweepControl ), NULL, this );
}

SoDaSNAFrame::~SoDaSNAFrame()
{
	// Disconnect Events
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnOpenConfig ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSaveConfig ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSaveConfigAs ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnCalibrate ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnQuit ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnAbout ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnUserGuide ) );
	m_SweepRate->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSweepSpeed ), NULL, this );
	m_CenterFreqBox->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( SoDaSNAFrame::OnFreqEnter ), NULL, this );
	m_CenterFreqBox->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( SoDaSNAFrame::OnFreqEnter ), NULL, this );
	m_CenterUnits->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_SpanMag->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_SpanUnits->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_OutPowerSliderA->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( SoDaSNAFrame::OnOutputPowerSel ), NULL, this );
	m_SpanMag1->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnFreqRangeSel ), NULL, this );
	m_SweepControl->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( SoDaSNAFrame::OnSweepControl ), NULL, this );
	
}

t_AboutDialog::t_AboutDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxVERTICAL );
	
	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("SoDaLogo_Big.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer57->Add( m_bitmap1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_staticText15 = new wxStaticText( this, wxID_ANY, wxT("SoDa Scalar Network Analyzer\nThe 'SoD' stands for Software Defined.\n The 'a' doesn't stand for anything.\n\n(Copyright 2014 kb1vc)\n  All rights reserved.\n\n  Redistribution and use in source and binary forms, with or without\n  modification, are permitted provided that the following conditions are\n  met:\n\n  Redistributions of source code must retain the above copyright\n  notice, this list of conditions and the following disclaimer.\n  Redistributions in binary form must reproduce the above copyright\n  notice, this list of conditions and the following disclaimer in\n  the documentation and/or other materials provided with the\n  distribution.\n\n  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n  \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
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
	m_AboutOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_AboutDialog::OnAboutOK ), NULL, this );
}

t_AboutDialog::~t_AboutDialog()
{
	// Disconnect Events
	m_AboutOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_AboutDialog::OnAboutOK ), NULL, this );
	
}

t_QuitDialogConfig::t_QuitDialogConfig( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
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
	m_QuitNSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_QuitDialogConfig::SaveConfigChanges ), NULL, this );
	m_QuitNoSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_QuitDialogConfig::OnIgnoreConfigChanges ), NULL, this );
}

t_QuitDialogConfig::~t_QuitDialogConfig()
{
	// Disconnect Events
	m_QuitNSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_QuitDialogConfig::SaveConfigChanges ), NULL, this );
	m_QuitNoSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_QuitDialogConfig::OnIgnoreConfigChanges ), NULL, this );
	
}

t_NewConfigDialog::t_NewConfigDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxVERTICAL );
	
	
	bSizer58->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText25 = new wxStaticText( this, wxID_ANY, wxT("SoDaBench could not find a configuration file.\nSelect \"OK\" to create a configuration file in\n${HOME}/.SoDaRadio/SoDaSNA.cfg"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
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
	m_CreateConfigDefault->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_NewConfigDialog::OnCreateConfigDefault ), NULL, this );
	m_NoThanksCreateConfigDefault->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_NewConfigDialog::OnDismissCreateConfigDefault ), NULL, this );
}

t_NewConfigDialog::~t_NewConfigDialog()
{
	// Disconnect Events
	m_CreateConfigDefault->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_NewConfigDialog::OnCreateConfigDefault ), NULL, this );
	m_NoThanksCreateConfigDefault->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_NewConfigDialog::OnDismissCreateConfigDefault ), NULL, this );
	
}

t_CalibrateDialog::t_CalibrateDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bProcessBox;
	bProcessBox = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxVERTICAL );
	
	m_CalStepTxt = new wxStaticText( this, wxID_ANY, wxT("Calibration Action Prompt goes here.\nand it takes two lines to do it.\nMaybe even three."), wxDefaultPosition, wxDefaultSize, 0 );
	m_CalStepTxt->Wrap( -1 );
	m_CalStepTxt->SetFont( wxFont( 12, 70, 90, 92, false, wxEmptyString ) );
	
	bSizer31->Add( m_CalStepTxt, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_CalStepPrompt2 = new wxStaticText( this, wxID_ANY, wxT("Press \"Done\" when action is complete."), wxDefaultPosition, wxDefaultSize, 0 );
	m_CalStepPrompt2->Wrap( -1 );
	m_CalStepPrompt2->SetFont( wxFont( 12, 70, 90, 92, false, wxEmptyString ) );
	
	bSizer31->Add( m_CalStepPrompt2, 0, wxALL, 5 );
	
	bSizer14->Add( bSizer31, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 5 );
	
	m_CalStepDone = new wxButton( this, wxID_ANY, wxT("Done"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_CalStepDone, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	bProcessBox->Add( bSizer14, 1, wxEXPAND, 5 );
	
	m_CalStatus = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP|wxVSCROLL );
	m_CalStatus->SetFont( wxFont( 12, 70, 90, 92, false, wxEmptyString ) );
	
	bProcessBox->Add( m_CalStatus, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 0 );
	
	bSizer12->Add( bProcessBox, 3, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer32->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_AbortCalibration = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_AbortCalibration, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer32->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_SaveCal = new wxButton( this, wxID_ANY, wxT("Save Calibration"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer32->Add( m_SaveCal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer32->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bSizer12->Add( bSizer32, 1, wxEXPAND, 5 );
	
	this->SetSizer( bSizer12 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_CalStepDone->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_CalibrateDialog::OnCalStepDone ), NULL, this );
	m_AbortCalibration->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_CalibrateDialog::OnAbortCalibration ), NULL, this );
	m_SaveCal->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_CalibrateDialog::OnSaveCalibration ), NULL, this );
}

t_CalibrateDialog::~t_CalibrateDialog()
{
	// Disconnect Events
	m_CalStepDone->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_CalibrateDialog::OnCalStepDone ), NULL, this );
	m_AbortCalibration->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_CalibrateDialog::OnAbortCalibration ), NULL, this );
	m_SaveCal->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( t_CalibrateDialog::OnSaveCalibration ), NULL, this );
	
}
