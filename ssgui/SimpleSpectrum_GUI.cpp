///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 25 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "SimpleSpectrum_GUI.h"

///////////////////////////////////////////////////////////////////////////
using namespace SoDaRadio_GUI;

SimpleSpectrumFrame::SimpleSpectrumFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menubar1 = new wxMenuBar( 0 );
	FileMenu = new wxMenu();
	wxMenuItem* Save;
	Save = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Save") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( Save );
	
	wxMenuItem* Quit;
	Quit = new wxMenuItem( FileMenu, wxID_ANY, wxString( wxT("Quit") ) , wxEmptyString, wxITEM_NORMAL );
	FileMenu->Append( Quit );
	
	m_menubar1->Append( FileMenu, wxT("&File") ); 
	
	HelpMenu = new wxMenu();
	wxMenuItem* Aboutitem;
	Aboutitem = new wxMenuItem( HelpMenu, wxID_ANY, wxString( wxT("About") ) , wxEmptyString, wxITEM_NORMAL );
	HelpMenu->Append( Aboutitem );
	
	m_menubar1->Append( HelpMenu, wxT("Help") ); 
	
	this->SetMenuBar( m_menubar1 );
	
	m_ClueBar = this->CreateStatusBar( 3, wxST_SIZEGRIP, wxID_ANY );
	wxBoxSizer* TopSizer;
	TopSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxHORIZONTAL );
	
	WaterFallPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer51->Add( WaterFallPanel, 1, wxEXPAND | wxALL, 5 );
	
	TopSizer->Add( bSizer51, 5, wxEXPAND, 5 );
	
	wxBoxSizer* ControlSizer;
	ControlSizer = new wxBoxSizer( wxHORIZONTAL );
	
	
	ControlSizer->Add( 20, 0, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbTxFreq;
	sbTxFreq = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("TX Frequency") ), wxVERTICAL );
	
	sbTxFreq->SetMinSize( wxSize( 230,-1 ) ); 
	m_TXFreqText = new wxStaticText( this, wxID_ANY, wxT("10,368.100 000"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_TXFreqText->Wrap( -1 );
	m_TXFreqText->SetFont( wxFont( 20, 70, 90, 90, false, wxEmptyString ) );
	
	sbTxFreq->Add( m_TXFreqText, 0, wxALL, 5 );
	
	ControlSizer->Add( sbTxFreq, 0, wxEXPAND, 5 );
	
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
	
	ControlSizer->Add( sbWfSettingsSizer, 0, wxBOTTOM|wxEXPAND, 15 );
	
	TopSizer->Add( ControlSizer, 1, 0, 5 );
	
	this->SetSizer( TopSizer );
	this->Layout();
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( SimpleSpectrumFrame::OnClose ) );
	this->Connect( Save->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SimpleSpectrumFrame::OnOpenLogfile ) );
	this->Connect( Quit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SimpleSpectrumFrame::OnQuit ) );
	this->Connect( Aboutitem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SimpleSpectrumFrame::OnAbout ) );
	WaterFallPanel->Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SimpleSpectrumFrame::OnOpenSpectConfig ), NULL, this );
	m_WaterfallWindowSel->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SimpleSpectrumFrame::OnWindowLenUpdate ), NULL, this );
	m_WaterfallScrollSpeed->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SimpleSpectrumFrame::OnScrollSpeedUpdate ), NULL, this );
}

SimpleSpectrumFrame::~SimpleSpectrumFrame()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( SimpleSpectrumFrame::OnClose ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SimpleSpectrumFrame::OnOpenLogfile ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SimpleSpectrumFrame::OnQuit ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SimpleSpectrumFrame::OnAbout ) );
	WaterFallPanel->Disconnect( wxEVT_MIDDLE_UP, wxMouseEventHandler( SimpleSpectrumFrame::OnOpenSpectConfig ), NULL, this );
	m_WaterfallWindowSel->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SimpleSpectrumFrame::OnWindowLenUpdate ), NULL, this );
	m_WaterfallScrollSpeed->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( SimpleSpectrumFrame::OnScrollSpeedUpdate ), NULL, this );
	
}

m_AboutDialog::m_AboutDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxVERTICAL );
	
	m_bitmap1 = new wxStaticBitmap( this, wxID_ANY, wxBitmap( wxT("SoDaLogo_Big.png"), wxBITMAP_TYPE_ANY ), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer57->Add( m_bitmap1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_staticText15 = new wxStaticText( this, wxID_ANY, wxT("SImple Spectrum Display\n\n(Copyright 2016 kb1vc)\n  All rights reserved.\n\n Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n\n Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n\n THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticText15->Wrap( 750 );
	m_staticText15->SetMinSize( wxSize( 775,-1 ) );
	
	bSizer57->Add( m_staticText15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_GUIVersion = new wxStaticText( this, wxID_ANY, wxT("GUI Version: "), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_GUIVersion->Wrap( -1 );
	bSizer57->Add( m_GUIVersion, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_SDRVersion = new wxStaticText( this, wxID_ANY, wxT("SDR Version:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
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
