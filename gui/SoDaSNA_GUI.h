///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jul  5 2013)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __SoDaSNA_GUI__
#define __SoDaSNA_GUI__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

namespace SoDaSNA_GUI
{
	///////////////////////////////////////////////////////////////////////////////
	/// Class SoDaSNAFrame
	///////////////////////////////////////////////////////////////////////////////
	class SoDaSNAFrame : public wxFrame 
	{
		private:
		
		protected:
			wxMenuBar* m_menubar1;
			wxMenu* FileMenu;
			wxMenu* HelpMenu;
			wxPanel* m_DisplayPanel;
			wxChoice* m_SweepRate;
			wxTextCtrl* m_CenterFreqBox;
			wxChoice* m_CenterUnits;
			wxChoice* m_SpanMag;
			wxChoice* m_SpanUnits;
			wxSlider* m_OutPowerSliderA;
			wxRadioBox* m_RFOutEna_A;
			wxSpinCtrl* m_RefLevel;
			wxChoice* m_SpanMag1;
			wxStaticText* m_staticText11;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnOpenConfig( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveConfig( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveConfigAs( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnCalibrate( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnUserGuide( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSweepSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFreqEnter( wxMouseEvent& event ) { event.Skip(); }
			virtual void OnFreqEnter( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnFreqRangeSel( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOutputPowerSel( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnOutputEna( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			SoDaSNAFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SoDa Scalar Network Analyzer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1185,850 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			~SoDaSNAFrame();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class t_AboutDialog
	///////////////////////////////////////////////////////////////////////////////
	class t_AboutDialog : public wxDialog 
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
			
			t_AboutDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 626,768 ), long style = wxDEFAULT_DIALOG_STYLE );
			~t_AboutDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class t_QuitDialogConfig
	///////////////////////////////////////////////////////////////////////////////
	class t_QuitDialogConfig : public wxDialog 
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
			
			t_QuitDialogConfig( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Save Configuration Changes?"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
			~t_QuitDialogConfig();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class t_NewConfigDialog
	///////////////////////////////////////////////////////////////////////////////
	class t_NewConfigDialog : public wxDialog 
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
			
			t_NewConfigDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Create a Default Configuration?"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 487,273 ), long style = wxDEFAULT_DIALOG_STYLE );
			~t_NewConfigDialog();
		
	};
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class t_CalibrateDialog
	///////////////////////////////////////////////////////////////////////////////
	class t_CalibrateDialog : public wxDialog 
	{
		private:
		
		protected:
			wxStaticText* m_CalStepTxt;
			wxStaticText* m_CalStepPrompt2;
			wxButton* m_CalStepDone;
			wxTextCtrl* m_CalStatus;
			
			wxButton* m_AbortCalibration;
			
			wxButton* m_SaveCal;
			
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnCalStepDone( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAbortCalibration( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnSaveCalibration( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			t_CalibrateDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SNA Calibration"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 610,501 ), long style = wxDEFAULT_DIALOG_STYLE );
			~t_CalibrateDialog();
		
	};
	
} // namespace SoDaSNA_GUI

#endif //__SoDaSNA_GUI__
