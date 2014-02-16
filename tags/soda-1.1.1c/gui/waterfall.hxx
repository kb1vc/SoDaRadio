#ifndef WATERFALL_HDR_H
#define WATERFALL_HDR_H

/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <wx/wx.h>
#include <map>

using namespace std;

namespace SoDaRadio_GUI {
  
  class SoDaRadio_Top;

  class Waterfall : public wxPanel
  {
  public:
    Waterfall(wxPanel * parent, SoDaRadio_Top * _radio, int id,
	      const wxPoint & pos = wxDefaultPosition,
	      const wxSize & size = wxDefaultSize,
	      const int flags = 0xff
	      );

    static const int DRAW_LABEL = 0x1;
    static const int DRAW_LEGEND = 0x4;
    static const int DRAW_TITLE = 0x8;

  private:
    double * freq_buffer; /// List of frequencies in spectrum sample
    float * power_buffer; /// bucket power in each freq bin (dB)
    int spectrum_len; /// number of freq bins
    int spec_line_y_pos;
    int bitmap_y_pos;
    int bitmap_y_incr; 
  public:
    void RegisterBuffers(double * freqs, float * power, int len) {
      spectrum_len = len; 
      power_buffer = power;
      freq_buffer = freqs;
    }
  
  
    void SetTitle(const wxString & l) { title = l; Refresh(); }
    wxString & GetTitle() { return title; }

    void SetCenterFreq(double freq) {
      center_freq = freq;
      if(cf_scalefactor != 0.0) {
	xmin = sxmin + center_freq;
	xmax = sxmax + center_freq;
      }
      Refresh(); 
    }
    void SetCenterFreqTemplate(const wxString & l, double scale_factor = 1.0)  {
      cf_template = l;
      cf_scalefactor = scale_factor;
    }
  
    void SetXLabel(const wxString & l) { xlabel = l; Refresh(); }

    void SetXTicTemplate(const wxString & l, double scale_factor = 1.0) { x_template = l; x_scalefactor = scale_factor; }
    void SetXCenterTemplate(const wxString & l) { xc_template = l; }

    void SetScale(double _xmin, double _xmax)
    {
      xmin = _xmin; xmax = _xmax;
      sxmin = _xmin, sxmax = _xmax; 
      if(cf_scalefactor != 0.0) {
	xmin = sxmin + center_freq; 
	xmax = sxmax + center_freq; 
      }
      ReSize(); 
    }

    void GetScale(double & _xmin, double & _xmax, double & _ymin, double & _ymax)
    {
      _xmin = xmin;
      _xmax = xmax;
    }

  
    void Draw(bool redraw_background = false);
    void Print(const wxString & fname, const wxString & note, const wxString & rnote);
    void DrawNew();
  private:

    void DrawOnDC(wxDC & dc, bool redraw_background = false, const wxString & note = wxString(wxT("")));
    float AveragePower(int minidx, int maxidx);
    void DrawSpectrum(wxDC & dc);
    wxColour & PowerToHeatColor(float val, float avg);
  

    void OnSize(wxSizeEvent & event); 
    void OnPaint(wxPaintEvent & event);
    void OnMB1Down(wxMouseEvent & event);

    void ReSize();

    bool ScaleX(double x, int & xpos); 
    bool UnScaleX(const wxPoint & pt, double & x);
    
    void DrawLabels(wxDC & dc, const wxString & note);
    void DrawData(wxDC & dc);

    bool spectrum_data_ready; 
  
    void InterpolateColor(int idx, wxColour & from, wxColour & to, wxColour & res);
    void InitHeatMap();

    static const int HEATMAP_SIZE = 256; 
    wxColour heatmap[HEATMAP_SIZE]; /// 256 shades of color seems reasonable.
    static const float heat_range = 30.0;
    static const float heat_floor = -5.0; 
    wxBitmap * bitmap;
    wxMemoryDC bitmapDC; 
    wxPanel * m_parent;
    SoDaRadio_Top * radio; 

    wxString title, xlabel;
    wxString x_template, xc_template;
    double x_scalefactor;

    double center_freq;
    wxString cf_template;
    double cf_scalefactor; 

    // dimensions
    // all derived from the widget size. 
    int width, height, graph_width, graph_height, spec_height;
    wxPoint ll, ur; 

    // scaling
    double xmin, xmax, ymin, ymax;  // supplied as parms. 
    double x_scale, y_scale;        // derived from parms and siz.
    double sxmin, sxmax;            // derived from parms and center freq

    // init -- inhibit actions until we're more or less ready
    bool enable_draw_events; 
  
  };
}
#endif
