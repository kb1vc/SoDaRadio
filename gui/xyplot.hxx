#ifndef XYPLOT_HDR_H
#define XYPLOT_HDR_H

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
#include "GraphClient.hxx"

namespace SoDaRadio_GUI {
  using namespace std;

  class XYPlot : public wxPanel
  {
  public:
    XYPlot(wxPanel * parent, GraphClient * _client, int id,
	   const wxPoint & pos = wxDefaultPosition,
	   const wxSize & size = wxDefaultSize,
	   const int flags = 0xff
	   );

    static const int DRAW_LABEL = 0x1;
    static const int DRAW_MARKERS = 0x2;
    static const int DRAW_LEGEND = 0x4;
    static const int DRAW_TITLE = 0x8;
    static const int DRAW_VERT_MARKER_BANDS = 0x10;
  
    class Trace {
    public:
      Trace(double * xlocs, float * ylocs, int _len,
	    int _stride = 1,
	    const wxString & trace_name = wxT("")); 

      ~Trace(); 
      void SetName(const wxString & trace_name) { name = trace_name; }

      void SetColor(const wxColor & c) {
	color = c;
	pen.SetColour(c); 
      }
      void SetRegion(const wxRect & r) { rect = r; }

      void MarkTrace(int marker_idx, int x, int y, XYPlot * plt);

      void SetLength(int _len) {
	len = _len; 
      }
      int GetLength() { return len; }

      double * GetXVec() { return xvec; }
      float * GetYVec() { return yvec; }
    
      friend class XYPlot; 
    protected:
      wxString name;
      double * xvec;
      float * yvec;
      int len;
      int stride;

      // index of marker value
      int markers[2];
    
      // for detection of mouse button hits
      wxRect rect;
      wxRect marker_rect[2]; 

      // for rendering
      wxColor color;
      wxPen pen; 
    };


  
    void AddTrace(int trace_ID, const wxColour & color, Trace * xytp);
    void DelTrace(int trace_ID); 
  
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
      Refresh(); 
    }
  
    void SetXLabel(const wxString & l) { xlabel = l; Refresh(); }
    void SetYLabel(const wxString & l) { ylabel = l; Refresh(); }

    void SetXTicTemplate(const wxString & l, double scale_factor = 1.0) { x_template = l; x_scalefactor = scale_factor; }
    void SetXCenterTemplate(const wxString & l) { xc_template = l; }
    void SetYTicTemplate(const wxString & l, double scale_factor = 1.0) { y_template = l; y_scalefactor = scale_factor; }

    void SetMarkerTemplate(const wxString & l,
			   double x_scale_factor = 1.0, double y_scale_factor = 1.0) {
      marker_template = l;
      marker_x_scalefactor = x_scale_factor;
      marker_y_scalefactor = y_scale_factor;
    }
  
    void SetScale(double _xmin, double _xmax, double _ymin, double _ymax)
    {
      xmin = _xmin; xmax = _xmax; ymin = _ymin; ymax = _ymax;
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
      _ymin = ymin;
      _ymax = ymax; 
    }
  
    void Draw();
  private:

    void OnSize(wxSizeEvent & event); 
    void OnPaint(wxPaintEvent & event);
    void OnMB1Down(wxMouseEvent & event);

    void ReSize();

    bool ScaleXY(double x, double y, wxPoint & pt);
    bool UnScaleXY(const wxPoint & pt, double & x, double & y); 
    
    void DrawGrid(wxDC & dc); 
    void DrawLabels(wxDC & dc);
    void DrawData(wxDC & dc);

    void DrawTrace(Trace * tr, wxDC & dc);
    void DrawMarker(int marker_idx, double x, double y, wxDC &dc); 

    void PrintXY(wxString & str, double x, double y);
  
    wxPanel * m_parent;
    GraphClient * client; 

    wxString title, xlabel, ylabel;
    wxString x_template, xc_template, y_template;
    double x_scalefactor, y_scalefactor;

    wxString marker_template;
    double marker_x_scalefactor, marker_y_scalefactor; 

    double center_freq;
    wxString cf_template;
    double cf_scalefactor; 

    // dimensions
    // all derived from the widget size. 
    int width, height, graph_width, graph_height;
    wxPoint ll, ur; 

    // scaling
    double xmin, xmax, ymin, ymax;  // supplied as parms. 
    double x_scale, y_scale;        // derived from parms and siz.
    double sxmin, sxmax;            // derived from parms and center freq


    // list of traces
    map<int, Trace *> traces;
    // the currently "selected" trace -- used for mouse events.
    Trace * selected_trace;
    // the currently selected marker
    int selected_marker; 

  };
}
#endif
