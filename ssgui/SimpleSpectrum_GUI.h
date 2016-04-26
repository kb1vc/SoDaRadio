///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 25 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __SimpleSpectrum_GUI__
#define __SimpleSpectrum_GUI__

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
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/slider.h>
#include <wx/frame.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

namespace SoDaRadio_GUI
{
	///////////////////////////////////////////////////////////////////////////////
	/// Class SimpleSpectrumFrame
	///////////////////////////////////////////////////////////////////////////////
	class SimpleSpectrumFrame : public wxFrame 
	{
		private:
		
		protected:
			wxMenuBar* m_menubar1;
			wxMenu* FileMenu;
			wxMenu* HelpMenu;
			wxStatusBar* m_ClueBar;
			wxPanel* WaterFallPanel;
			
			wxStaticText* m_TXFreqText;
			wxSlider* m_WaterfallWindowSel;
			
			wxSlider* m_WaterfallScrollSpeed;
			
			// Virtual event handlers, overide them in your derived class
			virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
			virtual void OnOpenLogfile( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
			virtual void OnOpenSpectConfig( wxMouseEvent& event ) { event.Skip(); }
			virtual void OnWindowLenUpdate( wxScrollEvent& event ) { event.Skip(); }
			virtual void OnScrollSpeedUpdate( wxScrollEvent& event ) { event.Skip(); }
			
		
		public:
			
			SimpleSpectrumFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SImpleSpectrum "), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1280,810 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			~SimpleSpectrumFrame();
		
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
	
} // namespace SoDaRadio_GUI

#endif //__SimpleSpectrum_GUI__
