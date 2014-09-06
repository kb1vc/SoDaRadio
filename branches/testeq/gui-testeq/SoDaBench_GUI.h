///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec 21 2009)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __SoDaBench_GUI__
#define __SoDaBench_GUI__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/frame.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/gbsizer.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

namespace SoDaBench_GUI
{
	///////////////////////////////////////////////////////////////////////////////
	/// Class SoDaBenchFrame
	///////////////////////////////////////////////////////////////////////////////
	class SoDaBenchFrame : public wxFrame 
	{
		private:
		
		protected:
			wxMenuBar* m_menubar1;
			wxMenu* FileMenu;
			wxMenu* HelpMenu;
			wxCheckBox* m_SAA;
			wxCheckBox* m_SAB;
			wxCheckBox* m_SweepA;
			wxCheckBox* m_SweepB;
			wxCheckBox* m_VNA;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnOpenConfig( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveConfig( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveConfigAs( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnUserGuide( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnInstSel( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			SoDaBenchFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SoDa RF Test Bench"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 229,259 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			~SoDaBenchFrame();
		
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
			
			m_AboutDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 626,768 ), long style = wxDEFAULT_DIALOG_STYLE );
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
	/// Class m_SweeperDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_SweeperDialog : public wxDialog 
	{
		private:
		
		protected:
			wxRadioBox* m_SweepMode;
			wxRadioBox* m_Modulation;
			wxRadioBox* m_RefSel;
			wxRadioBox* m_RFOutEna;
			wxTextCtrl* m_StartFreqBox;
			wxChoice* m_StartUnits;
			wxTextCtrl* m_StopFreqBox;
			wxChoice* m_StopUnits;
			wxChoice* m_SweepRate;
			wxSlider* m_OutPowerSlider;
			wxTextCtrl* m_CurFreqDisp;
			wxChoice* m_DispUnits;
			wxTextCtrl* m_CenterFreqBox;
			wxChoice* m_StopUnits1;
			wxTextCtrl* m_SpanFreqBox;
			wxChoice* m_StopUnits2;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnModeSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnModulationSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnRefSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOutputEna( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFreqEnter( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFreqRangeSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFreqSel( wxMouseEvent& event ) { event.Skip(); }
			virtual void OnSweepSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOutputPowerSel( wxScrollEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_SweeperDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Sweep Oscillator Output 0"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 433,401 ), long style = wxDEFAULT_DIALOG_STYLE );
			~m_SweeperDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class m_SpectrumAnalyzerDialog
	///////////////////////////////////////////////////////////////////////////////
	class m_SpectrumAnalyzerDialog : public wxDialog 
	{
		private:
		
		protected:
			wxTextCtrl* m_CenterFreqBox;
			wxChoice* m_CentUnits;
			wxChoice* m_SpanSelection;
			wxRadioBox* m_DispMode;
			wxChoice* m_ResBWCh;
			wxChoice* m_VidBWCh;
			wxSpinCtrl* m_RefLevelBox;
			wxChoice* m_choice15;
			wxChoice* m_VertRes;
			wxStaticText* m_VertResUnits;
			wxChoice* m_AntSelCh;
			wxSpinCtrl* m_inputAtten;
			wxStaticText* m_VertResUnits1;
			wxCheckBox* m_checkMaxTrace;
			wxCheckBox* m_checkAvgTrace;
			
			wxToggleButton* m_btnTraceReset;
			wxPanel* m_panel2;
			
			// Virtual event handlers, overide them in your derived class
			virtual void onCloseSpectrumAnalyzer( wxCloseEvent& event ) { event.Skip(); }
			virtual void onIconizeSpectrumAnalyzer( wxIconizeEvent& event ) { event.Skip(); }
			virtual void onInitSpectrumAnalyzer( wxInitDialogEvent& event ) { event.Skip(); }
			virtual void OnFreqEnter( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFreqRangeSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnDispModeSwitch( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnChoiceUpdate( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnRefLevel( wxSpinEvent& event ) { event.Skip(); }
			virtual void onInputAtten( wxSpinEvent& event ) { event.Skip(); }
			
		
		public:
			
			m_SpectrumAnalyzerDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Spectrum Analyzer: RX Channel A"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1207,586 ), long style = wxDEFAULT_DIALOG_STYLE );
			~m_SpectrumAnalyzerDialog();
		
	};
	
} // namespace SoDaBench_GUI

#endif //__SoDaBench_GUI__
