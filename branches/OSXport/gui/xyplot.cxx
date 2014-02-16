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

#include "xyplot.hxx"
#include <wx/print.h>
#include "SoDaRadio_Top.h"

using namespace std;

namespace SoDaRadio_GUI {
  
#define TITLE_Y_OFFSET 0
#define TRACE_LABEL_Y_OFFSET (TITLE_Y_OFFSET + 10)
#define TRACE_MARKER_Y_OFFSET (TITLE_Y_OFFSET + 15)
#define TRACE_MARKER_Y_INCR 15

  // this was (TRACE_MARKER_Y_OFFSET + 3 * TRACE_MARKER_Y_INCR)
#define GRID_TOP_OFFSET 10
#define XLABEL_Y_OFFSET 12
#define CENTER_FREQ_Y_OFFSET 5
#define GRID_BOT_OFFSET (XLABEL_Y_OFFSET + 18)

#define YLABEL_X_OFFSET 12

#define TRACE_MARKER_X_INCR 8

#define X_BOXTIC(n) ((int) (((double) (n * graph_width)) * 0.1))
#define Y_BOXTIC(n) ((int) (((double) (n * graph_height)) * 0.1))
#define XM_BOXTIC(n) ((int) (((double) (n * graph_width)) * 0.02))
#define YM_BOXTIC(n) ((int) (((double) (n * graph_height)) * 0.02))

  void XYPlot::PrintXY(wxString & str, double x, double y)
  {
    str.Printf(marker_template, x * marker_x_scalefactor, y * marker_y_scalefactor); 
  }

  XYPlot::XYPlot(wxPanel * parent, SoDaRadio_Top * _radio, int id,
		 const wxPoint & pos,
		 const wxSize & size,
		 const int flags
		 )
    : wxPanel(parent, id, pos, size, wxSUNKEN_BORDER)
  {
    m_parent = parent;
    radio = _radio;

    xlabel = wxT("");
    ylabel = wxT("");
    x_template = wxT("%4.1f");
    xc_template = wxT("%12.0f");
    y_template = wxT("%3.0f");
    cf_template = wxT("[%g]");
    marker_template = wxT("%6.4f %6.4f"); 

    cf_scalefactor = 0.0;
    marker_x_scalefactor = marker_y_scalefactor = x_scalefactor = y_scalefactor = 1.0; 

    sxmin = xmin = ymin = 0.0; sxmax = xmax = ymax = 1.0;
    center_freq = 0.5; 

    selected_trace = NULL;
    selected_marker = 0; 

    Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(XYPlot::OnMB1Down)); 
    Connect(wxEVT_PAINT, wxPaintEventHandler(XYPlot::OnPaint));
    Connect(wxEVT_SIZE, wxSizeEventHandler(XYPlot::OnSize));
  
    ReSize(); 
  }

  void XYPlot::OnMB1Down(wxMouseEvent & event) {
    int x, y;
    x = event.GetX();
    y = event.GetY();
    bool refresh_required = false; 

    if((x > ll.x) && (x < ur.x) && (y < ll.y) && (y > ur.y)) {
      double fx, fy;
      UnScaleXY(wxPoint(x, y), fx, fy);
      radio->SetRXFreqFromDisp(fx); 
    }
    if (refresh_required) Refresh(); 
    event.Skip(); 
  }

  void XYPlot::OnSize(wxSizeEvent & event) {
    ReSize(); 
  }

  void XYPlot::DrawGrid(wxDC & dc)
  {
    // draw the box in black on a white background
    dc.SetPen(*wxMEDIUM_GREY_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);

    dc.DrawRectangle(3, 3, width-6, height-6);

    // draw vertical marks.
    int i, j;
    int x;
    int y;
    x = ll.x;
    y = ur.y;
  
    for(i = 0; i < 11; i++) {
      x = ll.x + X_BOXTIC(i);
      y = ur.y + Y_BOXTIC(i); 

      dc.DrawLine(x, ll.y, x, ur.y);
      dc.DrawLine(ll.x, y, ur.x, y);
    }

    // now draw hashes
    x = ll.x + X_BOXTIC(5) - 3;
    y = ur.y + Y_BOXTIC(5) - 3;
    int xtic, ytic; 
    for(i = 0; i < 50; i++) {
      xtic = ll.x + XM_BOXTIC(i); // (int) (((double) (i * x_boxtic) / 5.0);
      ytic = ur.y + YM_BOXTIC(i); // (int) (((double) (i * y_boxtic)) / 5.0);
      dc.DrawLine(x, ytic, x+6, ytic);
      dc.DrawLine(xtic, y, xtic, y+6);
    }
  }

  void XYPlot::Draw()
  {
    // setup the panel with the DC.
    wxPaintDC dc(this);

    // how big is the window?
    wxSize size = GetSize();
    int width = size.GetWidth();
    int height = size.GetHeight();
  

    // Draw the grid.
    DrawGrid(dc);

    // Draw the Title and text
    DrawLabels(dc);

    // now draw the traces
    DrawData(dc);

  }

  bool XYPlot::UnScaleXY(const wxPoint & pt, double & x, double & y)
  {
    double ix, iy;
    ix = (double) (pt.x - ll.x);
    iy = (double) (ll.y - pt.y);

    x = xmin + ix / x_scale;
    y = ymin + iy / y_scale;
  
    return true; 
  }

  bool XYPlot::ScaleXY(double x, double y, wxPoint & pt) {
    // fix out-of-range.
    // we have to be really careful about things that are way
    // out of bounds....
    bool ret = true; 
    long lx = ( long) ((x - xmin) * x_scale);
    int ix;
    if(x < xmin) return false;
    if(x > xmax) return false;
  
    if (lx > 10000L) {
      ix = 10000;
      ret = false; 
    }
    else {
      ix = (int) (lx);
    }
    if (lx < 0L) {
      ix = -1000;
      ret = false; 
    }

    pt.x = ll.x + ix; 
    pt.y = ll.y - (int) ((y - ymin) * y_scale);

    return ret; 
  }

  wxPoint MarkerPolygons[2][5] = {
    {
      wxPoint(-4, 4), wxPoint(4, 4), wxPoint(0, -4),
      wxPoint(0, -4), wxPoint(0,-4) 
    },
    {
      wxPoint(-4, -4), wxPoint(4, -4), wxPoint(4, 4),
      wxPoint(-4, 4), wxPoint(-4, 4) 
    } 
  };

  void XYPlot::DrawMarker(int marker_idx, double x, double y, wxDC &dc)
  {
    int idx = marker_idx % 2;
    wxPoint pt;

    bool onscr = ScaleXY(x, y, pt);
    if(pt.x < ll.x) pt.x = ll.x;
    if(pt.x > ur.x) pt.x = ur.x;
    if(pt.y < ur.y) pt.y = ur.y; 
    if(pt.y > ll.y) pt.y = ll.y; 

    dc.DrawPolygon(5, MarkerPolygons[idx], pt.x, pt.y);
  }

  void XYPlot::DrawTrace(Trace * tr, wxDC & dc)
  {
    int i;
    double *xp = tr->xvec + tr->stride;
    float *yp = tr->yvec + tr->stride;
    wxPoint last_pt, new_pt;
    bool last_good, new_good; 
    wxDCClipper clip(dc, wxRect(ll, ur));
  
    dc.SetPen(tr->pen);

    last_good = ScaleXY(*(tr->xvec), *(tr->yvec), last_pt);
  
    for(i = 1; i < tr->len; i++) {
      new_good = ScaleXY(*xp, *yp, new_pt);
      if (new_good || last_good) {
	dc.DrawLine(last_pt, new_pt);
      }
      last_pt = new_pt; 
      xp += tr->stride;
      yp += tr->stride; 
    }
  }

  void XYPlot::DrawData(wxDC & dc)
  {
    // iterate through each trace and draw it.
    map<int, Trace *>::const_iterator itr;
    for(itr = traces.begin(); itr != traces.end(); ++itr) {
      DrawTrace(itr->second, dc); 

      // any markers?  we draw these here (rather than in DrawTrace
      // because DrawTrace is done in a clipping region.  We want
      // the marker to show when it is on a boundary. 
      Trace * tr = itr->second;
      int i;
      for(i = 0; i < 2; i++) {
	int idx = tr->markers[i] * tr->stride;
	if(idx < 0) continue; 
	DrawMarker(i, tr->xvec[idx], tr->yvec[idx], dc); 
      }
    }
  }

  void XYPlot::DrawLabels(wxDC & dc)
  {
    wxFont tfont(12, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
    wxFont lfont(10, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
    wxFont rfont(8, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);

    dc.SetFont(tfont);
    dc.SetTextForeground(wxColour(0xff, 0xff, 0xff));
    dc.SetBackgroundMode(wxTRANSPARENT);

    wxPoint cpt;
    wxCoord w, h; 
  
    dc.SetFont(lfont);

    // Draw the center frequency
    if(cf_scalefactor != 0.0) {
      cpt.x = ll.x;
      cpt.y = height - CENTER_FREQ_Y_OFFSET;
      wxString cfstr;
      cfstr.Printf(cf_template, center_freq * cf_scalefactor);
      dc.GetTextExtent(cfstr, &w, &h);
      dc.DrawText(cfstr, cpt.x - h, cpt.y);
    }
  
    // Draw the xlabel
    cpt.x = (ll.x + ur.x) / 2;
    cpt.y = height - XLABEL_Y_OFFSET; 
    dc.GetTextExtent(xlabel, &w, &h);
    dc.DrawText(xlabel, cpt.x - w/2, cpt.y); 

    // Draw the ylabel
    cpt.y = (ll.y + ur.y) / 2;
    cpt.x = ll.x - YLABEL_X_OFFSET; 
    dc.GetTextExtent(ylabel, &w, &h);
    dc.DrawRotatedText(ylabel, cpt.x - h/2, cpt.y + w/2, 90); 

    double v, vincr;
    wxString tlab;
    int i; 
    dc.SetFont(rfont);
    // Draw the xStart, xMid, and xEnd
    v = sxmin;
    //  vincr = (xmax - xmin) / 2.0;
    vincr = (sxmax - sxmin) / 10.0;
    double vmid = sxmin + vincr * 5.0; 
    cpt.x = ll.x;
    cpt.y = ll.y;

    for(i = 0; i < 11; i++, v += vincr) {
      cpt.x = ll.x + X_BOXTIC(i);
      if(i == 5) {
	tlab.Printf(xc_template, v * x_scalefactor);
      }
      else {
	tlab.Printf(x_template, (v - vmid) * x_scalefactor);
      }
      tlab = tlab.Trim(true).Trim(false);
      dc.GetTextExtent(tlab, &w, &h);
      dc.DrawText(tlab, cpt.x - w/2, cpt.y + h/2); 
    }

    // Draw the Y values up the axis.
    v = ymin;
    vincr = (ymax - ymin) / 10.0;
    cpt.x = ur.x + 3;
    cpt.y = ll.y; 
    for(i = 0; i < 11; i++, v += vincr) {
      cpt.y = ll.y - Y_BOXTIC(i);
      tlab.Printf(y_template, v * y_scalefactor);
      tlab = tlab.Trim(true).Trim(false);
      dc.GetTextExtent(tlab, &w, &h);
      dc.DrawText(tlab, cpt.x, cpt.y - h / 2); 
    }

    // Draw the keys.
    cpt.x = ll.x;
    cpt.y = ur.y;
    dc.SetFont(lfont);
    map<int, Trace *>::const_iterator itr;
    for(itr = traces.begin(); itr != traces.end(); ++itr) {
      cpt.y = TRACE_LABEL_Y_OFFSET;
      Trace * tr = itr->second;
      dc.GetTextExtent(tr->name, &w, &h);
      dc.SetTextForeground(tr->color);
      if(tr == selected_trace) {
	dc.SetTextBackground(wxColour(0x40, 0x40, 0x40, 0x80)); 
	dc.SetBackgroundMode(wxSOLID);
      }
      else {
	dc.SetBackgroundMode(wxTRANSPARENT);
      }
      dc.DrawText(tr->name, cpt.x, cpt.y - h);
      tr->SetRegion(wxRect(cpt.x, cpt.y - h, w, h));
      // bump past the name...
      cpt.x += w + 15; 
      // now draw the two markers.
      cpt.y = TRACE_MARKER_Y_OFFSET;
      int maxw = 0; 
      wxString xylab;
      for(i = 0 ; i < 2; i++) {
	// if this is the selected marker, then draw it in the
	// trace color. otherwise use grey.
	dc.SetBackgroundMode(wxTRANSPARENT);
	if(selected_marker == i) {
	  dc.SetPen(tr->pen); 
	}
	else {
	  dc.SetPen(*wxMEDIUM_GREY_PEN);
	}
	// first draw the symbol
	dc.DrawPolygon(5, MarkerPolygons[i], cpt.x, cpt.y); 
	dc.SetPen(tr->pen);
	w = 0;
	// then the XY values
	if(tr->markers[i] >= 0) {
	  PrintXY(xylab, tr->xvec[tr->markers[i] * tr->stride], tr->yvec[tr->markers[i] * tr->stride]);
	  dc.GetTextExtent(xylab, &w, &h);
	  dc.DrawText(xylab, cpt.x + 24, cpt.y - h/2);

	  if(w > maxw) maxw = w; 
	}
	// now setup the marker rectangle.
	tr->marker_rect[i] = wxRect(cpt.x - 4, cpt.y - 4, cpt.x + w + 24, cpt.y + h/2);
      
	// now bump 
	cpt.y += TRACE_MARKER_Y_INCR;
      }
    
      // now draw the delta between the two markers.
      dc.SetPen(*wxMEDIUM_GREY_PEN);
      dc.DrawPolygon(5, MarkerPolygons[1], cpt.x, cpt.y);
      cpt.x += 8;
      dc.DrawLine(cpt.x, cpt.y, cpt.x + 4, cpt.y); 
      cpt.x += 8;
      dc.DrawPolygon(5, MarkerPolygons[0], cpt.x, cpt.y);
      dc.SetPen(tr->pen); 
      if((tr->markers[0] >= 0) && (tr->markers[1] >= 0)) {
	// draw the difference.
	double dx, dy;
	dx = tr->xvec[tr->markers[1] * tr->stride] - 
	  tr->xvec[tr->markers[0] * tr->stride]; 
	dy = tr->yvec[tr->markers[1] * tr->stride] - 
	  tr->yvec[tr->markers[0] * tr->stride]; 
	PrintXY(xylab, dx, dy); 
	dc.GetTextExtent(xylab, &w, &h);
	dc.DrawText(xylab, cpt.x + 8, cpt.y - h/2);
	if(w > maxw) maxw = w; 
      }
      cpt.x += maxw + 24 + 10; 
    }
  }


  void XYPlot::ReSize()
  {
    // This is a good time to recalculate things like
    // the tic arrays.
    // how big is the guage?
    wxSize size = GetSize();
    width = size.GetWidth();
    height = size.GetHeight();

    ll.x = 40; 
    ur.x = width - 60; 

    ur.y = GRID_TOP_OFFSET;
    ll.y = height - GRID_BOT_OFFSET;


    graph_width = ur.x - ll.x; 
    graph_height = ll.y - ur.y;
    //  x_boxtic = graph_width / 10;
    //  y_boxtic = graph_height / 10; 


    x_scale = ((double) graph_width) / (xmax - xmin);
    y_scale = ((double) graph_height) / (ymax - ymin);
    // Refresh(); 
  }

  void XYPlot::OnPaint(wxPaintEvent & event)
  {
    Draw(); 
  }

  void XYPlot::AddTrace(int trace_ID, const wxColour & color, Trace * xytp)
  {
    xytp->color = color;
    xytp->pen = wxPen(color, 3);
    traces[trace_ID] = xytp;
    selected_trace = xytp;
  }

  void XYPlot::DelTrace(int trace_ID)
  {
    traces.erase(trace_ID);
  }

  XYPlot::Trace::Trace(double * xlocs, float * ylocs, int _len, int _stride, 
		       const wxString & trace_name)
  {
    name = trace_name;
    xvec = xlocs;
    yvec = ylocs;
    len = _len;
    stride = _stride;
    markers[0] = markers[1] = -1; 
  }


  void XYPlot::Trace::MarkTrace(int marker_num, int x, int y, XYPlot * plt)
  {
    int i;
    double *tx = xvec;
    float *ty = yvec;
    int best_idx = -1;
    int best_sqdist;

    // only two markers. 
    marker_num = marker_num % 2;
  
    // we need to do the trace marking in the display coordinate
    // space, since axes aren't similarly scaled, computing distances
    // in the data space gives surprising answers.

    best_sqdist = 0x7fffffff;
    double dist;
    for(i = 0; i < len; i++) {
      int dx, dy;
      wxPoint ppt; 
      plt->ScaleXY(*tx, *ty, ppt); 
      dx = x - ppt.x;
      dy = y - ppt.y; 
      dist = dx * dx + dy * dy;
      if(dist < best_sqdist) {
	best_sqdist = dist;
	best_idx = i; 
      }
      tx += stride;
      ty += stride; 
    }

    if(best_idx >= 0) {
      markers[marker_num] = best_idx;
    }
    else {
      cerr << "Couldn't find good value for selected trace at " << x << "," << y << endl; 
    }
  }

  XYPlot::Trace::~Trace()
  {
  }
}
