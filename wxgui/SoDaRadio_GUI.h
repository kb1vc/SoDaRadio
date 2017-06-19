///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul  5 2013)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __SoDaRadio_GUI__
#define __SoDaRadio_GUI__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statusbr.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/grid.h>
#include <wx/dialog.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/bmpbuttn.h>
#include <wx/radiobox.h>

///////////////////////////////////////////////////////////////////////////

namespace SoDaRadio_GUI
{
	///////////////////////////////////////////////////////////////////////////////
	/// Class SoDaRadioFrame
	///////////////////////////////////////////////////////////////////////////////
	class SoDaRadioFrame : public wxFrame 
	{
		private:
		
		protected:
			enum
			{
				ID_GOTOCALL = 1000,
				ID_GOTOGRID,
				ID_GOTOLOG,
				ID_GOTOMSG,
				ID_TOGGLETX,
			};
			
			wxMenuBar* m_menubar1;
			wxMenu* FileMenu;
			wxMenu* ConfigMenu;
			wxMenu* m_QSOMenu;
			wxMenu* m_bandSelect;
			wxMenu* HelpMenu;
			wxStatusBar* m_ClueBar;
			wxNotebook* SpectrumDisplay;
			wxPanel* WaterFallPanel;
			wxPanel* FFTPanel;
			wxChoice* m_ModeBox;
			wxChoice* m_AFBWChoice;
			wxSlider* m_AFGain;
			wxSlider* m_RFGain;
			
			wxStaticText* m_TXFreqText;
			wxCheckBox* m_TXRXLocked;
			wxCheckBox* m_SpecTrack;
			wxStaticText* m_RXFreqText;
			wxButton* Tune;
			wxButton* m_RX2CF;
			wxButton* m_ConfigSpectrum;
			wxTextCtrl* m_CWTextOutbound;
			wxTextCtrl* m_CWTextEntry;
			wxStaticText* m_MyGrid;
			wxTextCtrl* m_ToGrid;
			wxTextCtrl* m_ToCall;
			wxStaticText* m_BearingText;
			wxStaticText* m_RevBearingText;
			wxStaticText* m_RangeText;
			wxButton* m_CWsendEx;
			wxButton* m_CWsendInfo;
			wxButton* m_CWsendCall;
			wxButton* m_CWsendGrid;
			wxButton* m_CWsendQSL;
			wxButton* m_CWsendBK;
			wxButton* m_CWsend73;
			wxButton* m_CWsendV;
			wxButton* m_CWsendCarrier;
			wxSpinCtrl* m_RepeatCount;
			wxButton* m_clrCWbutton;
			wxButton* m_button10;
			wxButton* m_EditLogButton;
			wxTextCtrl* m_LogCommentBox;
			wxButton* CtrlPopup;
			wxButton* m_PTT;
			wxStaticText* m_UTC;
			wxStaticText* m_GPSGrid;
			wxStaticText* m_GPSLat;
			wxStaticText* m_GPSLon;
			wxChoice* m_RxAntChoice;
			wxChoice* m_TxAntChoice;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			virtual void OnOpenConfig( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveConfig( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveConfigAs( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOpenLogfile( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSetFromCall( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSetFromGrid( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnGPSOnSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnConfigBand( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMenuConfigSpect( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnMenuConfigTXAudio( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnQSOMenuSet( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnUserGuide( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOpenSpectConfig( wxMouseEvent& event ) { event.Skip(); }
			virtual void OnSelectPage( wxNotebookEvent& event ) { event.Skip(); }
			virtual void OnPaintWaterfall( wxPaintEvent& event ) { event.Skip(); }
			virtual void OnWFallFreqSel( wxMouseEvent& event ) { event.Skip(); }
			virtual void OnPaintPeriodogram( wxPaintEvent& event ) { event.Skip(); }
			virtual void OnPeriodogramFreqSel( wxMouseEvent& event ) { event.Skip(); }
			virtual void OnModeChoice( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAFBWChoice( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAFGainScroll( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnRFGainScroll( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnTXRXLock( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnTunePopup( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnPerRxToCentFreq( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSendText( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnNewToGrid( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnNewToGridEnter( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnNewToCall( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnCWControl( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnClrCWBuffer( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnLogContact( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnEditLog( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveComment( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnCtrlPopup( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnTXOnOff( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAntChoice( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			SoDaRadioFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SoDa Radio (by kb1vc)"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1280,810 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			~SoDaRadioFrame();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_LogDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_LogDialog : public wxDialog 
	{
		private:
		
		protected:
			wxGrid* m_LogGrid;
			wxButton* m_LogOK;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnLogCellChange( wxGridEvent& event ) { event.Skip(); }
			virtual void OnLogOK( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_LogDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Contact Log"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1024,350 ), long style = wxDEFAULT_DIALOG_STYLE );
			~m_LogDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_AboutDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_AboutDialog : public wxDialog 
	{
		private:
		
		protected:
			wxStaticBitmap* m_bitmap1;
			wxStaticText* m_staticText15;
			wxStaticText* m_GUIVersion;
			wxStaticText* m_SDRVersion;
			wxStaticLine* m_staticline5;
			wxButton* m_AboutOK;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnAboutOK( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_AboutDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,768 ), long style = wxDEFAULT_DIALOG_STYLE );
			~m_AboutDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_QuitDialogConfig
	///////////////////////////////////////////////////////////////////////////////
	class m_QuitDialogConfig : public wxDialog 
	{
		private:
		
		protected:
			wxStaticText* m_staticText10;
			wxButton* m_QuitNSave;
			
			wxButton* m_QuitNoSave;
			
			// Virtual event handlers, overide them in your derived class
			virtual void SaveConfigChanges( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnIgnoreConfigChanges( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_QuitDialogConfig( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Save Configuration Changes?"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
			~m_QuitDialogConfig();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_SpectConfigDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_SpectConfigDialog : public wxDialog 
	{
		private:
		
		protected:
			wxChoice* m_BandSpreadChoice;
			
			wxSpinCtrl* m_cFreqSpin;
			wxSlider* m_PeriodogramWindowSel;
			wxChoice* m_dBScale;
			wxSpinCtrl* m_RefLevel;
			wxSlider* m_WaterfallWindowSel;
			
			wxSlider* m_WaterfallScrollSpeed;
			
			wxButton* m_OK;
			
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnDone( wxCloseEvent& event ) { event.Skip(); }
			virtual void OnPerBandSpread( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnPerCFreqStep( wxSpinEvent& event ) { event.Skip(); }
			virtual void OnWindowLenUpdate( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnPerYScaleChoice( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnPerRefLevel( wxSpinEvent& event ) { event.Skip(); }
			virtual void OnScrollSpeedUpdate( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnDone( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_SpectConfigDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Spectrum Display Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE );
			~m_SpectConfigDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_TuningDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_TuningDialog : public wxDialog 
	{
		private:
		
		protected:
			enum
			{
				DIG_UP = 1000,
			};
			
			wxBitmapButton* m_DigitUpT10;
			wxTextCtrl* digitTextT10;
			wxBitmapButton* m_DigitDownT10;
			wxBitmapButton* m_DigitUpT9;
			wxTextCtrl* digitTextT9;
			wxBitmapButton* m_DigitDownT9;
			wxStaticText* m_staticText8;
			wxBitmapButton* m_DigitUpT8;
			wxTextCtrl* digitTextT8;
			wxBitmapButton* m_DigitDownT8;
			wxBitmapButton* m_DigitUpT7;
			wxTextCtrl* digitTextT7;
			wxBitmapButton* m_DigitDownT7;
			wxBitmapButton* m_DigitUpT6;
			wxTextCtrl* digitTextT6;
			wxBitmapButton* m_DigitDownT6;
			wxStaticText* m_staticText811;
			wxBitmapButton* m_DigitUpT5;
			wxTextCtrl* digitTextT5;
			wxBitmapButton* m_DigitDownT5;
			wxBitmapButton* m_DigitUpT4;
			wxTextCtrl* digitTextT4;
			wxBitmapButton* m_DigitDownT4;
			wxBitmapButton* m_DigitUpT3;
			wxTextCtrl* digitTextT3;
			wxBitmapButton* m_DigitDownT3;
			wxStaticText* m_staticText81;
			wxBitmapButton* m_DigitUpT2;
			wxTextCtrl* digitTextT2;
			wxBitmapButton* m_DigitDownT2;
			wxBitmapButton* m_DigitUpT1;
			wxTextCtrl* digitTextT1;
			wxBitmapButton* m_DigitDownT1;
			wxBitmapButton* m_DigitUpT0;
			wxTextCtrl* digitTextT0;
			wxBitmapButton* m_DigitDownT0;
			wxButton* b_tx_2_rx;
			wxButton* b_last_tx;
			wxBitmapButton* m_DigitUpR10;
			wxTextCtrl* digitTextR10;
			wxBitmapButton* m_DigitDownR10;
			wxBitmapButton* m_DigitUpR9;
			wxTextCtrl* digitTextR9;
			wxBitmapButton* m_DigitDownR9;
			wxStaticText* m_staticText82;
			wxBitmapButton* m_DigitUpR8;
			wxTextCtrl* digitTextR8;
			wxBitmapButton* m_DigitDownR8;
			wxBitmapButton* m_DigitUpR7;
			wxTextCtrl* digitTextR7;
			wxBitmapButton* m_DigitDownR7;
			wxBitmapButton* m_DigitUpR6;
			wxTextCtrl* digitTextR6;
			wxBitmapButton* m_DigitDownR6;
			wxStaticText* m_staticText8111;
			wxBitmapButton* m_DigitUpR5;
			wxTextCtrl* digitTextR5;
			wxBitmapButton* m_DigitDownR5;
			wxBitmapButton* m_DigitUpR4;
			wxTextCtrl* digitTextR4;
			wxBitmapButton* m_DigitDownR4;
			wxBitmapButton* m_DigitUpR3;
			wxTextCtrl* digitTextR3;
			wxBitmapButton* m_DigitDownR3;
			wxStaticText* m_staticText812;
			wxBitmapButton* m_DigitUpR2;
			wxTextCtrl* digitTextR2;
			wxBitmapButton* m_DigitDownR2;
			wxBitmapButton* m_DigitUpR1;
			wxTextCtrl* digitTextR1;
			wxBitmapButton* m_DigitDownR1;
			wxBitmapButton* m_DigitUpR0;
			wxTextCtrl* digitTextR0;
			wxBitmapButton* m_DigitDownR0;
			wxButton* b_tx_2_rx1;
			wxButton* b_last_tx1;
			wxCheckBox* m_ExtRefEn;
			wxStaticLine* m_staticline1;
			wxStaticText* m_RefStatus;
			wxStaticLine* m_staticline2;
			wxStaticText* m_RXLOStatus1;
			wxStaticLine* m_staticline3;
			wxStaticText* m_TXLOStatus;
			wxStaticLine* m_staticline4;
			wxButton* m_ReSyncUWLO;
			wxButton* m_TuneDone;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnDigitUp( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnDigitDown( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnCopyTXtoRX( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnLastTX( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnCopyRXtoTX( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnLastRX( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnExtRefEna( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnTransvLOCal( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnTuningDone( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_TuningDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
			~m_TuningDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_ControlsDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_ControlsDialog : public wxDialog 
	{
		private:
		
		protected:
			wxSlider* m_TXPower;
			wxSlider* m_STGain;
			wxSlider* m_CWSpeed;
			wxButton* m_CtrlDone;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnTXPower( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnSTGainScroll( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnCWSpeed( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnCtrlDone( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_ControlsDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Controls"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
			~m_ControlsDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_NewConfigDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_NewConfigDialog : public wxDialog 
	{
		private:
		
		protected:
			
			wxStaticText* m_staticText25;
			
			wxStaticText* m_StatusInfo;
			
			
			wxButton* m_CreateConfigDefault;
			
			wxButton* m_NoThanksCreateConfigDefault;
			
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnCreateConfigDefault( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnDismissCreateConfigDefault( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_NewConfigDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Create a Default Configuration?"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 487,273 ), long style = wxDEFAULT_DIALOG_STYLE );
			~m_NewConfigDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_BandConfigDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_BandConfigDialog : public wxDialog 
	{
		private:
		
		protected:
			wxStaticText* m_staticText36;
			wxChoice* m_BandChoiceBox;
			wxTextCtrl* m_BandName;
			wxStaticText* m_staticText27;
			wxChoice* m_RXAntChoice;
			wxStaticText* m_staticText40;
			wxChoice* m_TXAntChoice;
			wxStaticText* m_staticText31;
			wxTextCtrl* m_low_edge;
			wxStaticText* m_staticText34;
			wxStaticText* m_staticText32;
			wxTextCtrl* m_high_edge;
			wxStaticText* m_staticText33;
			wxStaticText* m_staticText39;
			wxChoice* m_ModChoice;
			wxCheckBox* m_TXEna;
			wxStaticText* m_staticText41;
			wxSpinCtrl* m_BandID;
			wxCheckBox* m_TransverterMode;
			wxCheckBox* m_LOGenMode;
			wxRadioBox* m_InjectionSel;
			wxStaticText* m_TransFreqLabel;
			wxTextCtrl* m_TransFreqEntry;
			wxStaticText* m_TransFreqLabel2;
			wxStaticText* m_TransMultLabel;
			wxTextCtrl* m_TransMultEntry;
			wxStaticLine* m_staticline6;
			
			wxButton* m_BandCancel;
			
			wxButton* m_BandOK;
			
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnConfigChoice( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnTransverterModeSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnBandCancel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnBandOK( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_BandConfigDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Band Configuration"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
			~m_BandConfigDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_TXAudioDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_TXAudioDialog : public wxDialog 
	{
		private:
		
		protected:
			wxRadioBox* m_TXA_Source;
			wxStaticText* m_noiseWarning;
			wxCheckBox* m_TXA_FiltEna;
			wxStaticLine* m_staticline7;
			wxButton* m_TXA_OK;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnTXAudioSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnTXAudioFilterEnable( wxCommandEvent& event ) { event.Skip(); }
			virtual void closeWindow( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_TXAudioDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Transmit Audio Control"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
			~m_TXAudioDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_BandConfigProblem
	///////////////////////////////////////////////////////////////////////////////
	class m_BandConfigProblem : public wxDialog 
	{
		private:
		
		protected:
			wxStaticText* m_BrokenBandForm;
			wxStaticText* m_BandConfigReason;
			wxButton* m_button38;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnBandErrorOK( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_BandConfigProblem( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Error in Band Configuration Request"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 563,198 ), long style = wxDEFAULT_DIALOG_STYLE );
			~m_BandConfigProblem();
		
	};
	
} // namespace SoDaRadio_GUI

#endif //__SoDaRadio_GUI__
