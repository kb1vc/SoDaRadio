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

#include "waterfall.hxx"
#include <wx/print.h>
#include "SoDaRadio_Top.h"
#include <boost/format.hpp>
#include <math.h>

using namespace std; 

namespace SoDaRadio_GUI {
  
#define TITLE_Y_OFFSET 0


#define GRID_TOP_OFFSET 10
#define XLABEL_Y_OFFSET 20
#define CENTER_FREQ_Y_OFFSET 5
#define GRID_BOT_OFFSET (XLABEL_Y_OFFSET + 24)

#define X_BOXTIC(n) ((int) (((double) (n * graph_width)) * 0.1))
#define Y_BOXTIC(n) ((int) (((double) (n * graph_height)) * 0.1))
#define XM_BOXTIC(n) ((int) (((double) (n * graph_width)) * 0.02))
#define YM_BOXTIC(n) ((int) (((double) (n * graph_height)) * 0.02))

  const float Waterfall::heat_range = 30.0;
  const float Waterfall::heat_floor = -5.0;
  
  Waterfall::Waterfall(wxPanel * parent, SoDaRadio_Top * _radio, int id,
		       const wxPoint & pos,
		       const wxSize & size,
		       const int flags
		       )
    : wxPanel(parent, id, pos, size, wxSUNKEN_BORDER)
  {
    enable_draw_events = false; 
    m_parent = parent;
    radio = _radio;

    // when we get exposed, we'll set the bitmap size. 
    bitmap = NULL;
    // we don't have a connection to a radio yet... no incoming freq stuff... 
    freq_buffer = NULL; 

    spectrum_data_ready = false; 

    xlabel = wxT("Frequency (MHz/kHz)");
    x_template = wxT("%4.1f");
    xc_template = wxT("%12.0f");
    cf_template = wxT("%9.4f");

    cf_scalefactor = 0.0;
    x_scalefactor = 1.0; 

    sxmin = xmin = ymin = 0.0; sxmax = xmax = ymax = 1.0;
    center_freq = 150.0e6; 

    InitHeatMap(); 
    ReSize();
  
    Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(Waterfall::OnMB1Down)); 
    Connect(wxEVT_MIDDLE_UP, wxMouseEventHandler(Waterfall::OnMB2Up)); 
    Connect(wxEVT_PAINT, wxPaintEventHandler(Waterfall::OnPaint));
    Connect(wxEVT_SIZE, wxSizeEventHandler(Waterfall::OnSize));

    enable_draw_events = true; 
  }

  void Waterfall::OnMB2Up(wxMouseEvent & event) {
    radio->OnOpenSpectConfig(event);
  }
  
  void Waterfall::OnMB1Down(wxMouseEvent & event) {
    if(!enable_draw_events) return; 
    int x, y;
    x = event.GetX();
    y = event.GetY();
    bool refresh_required = false; 

    // debug: dump the scrolling counters
    // std::cerr << boost::format("slyp = %d byp = %d byi = %d spec_height = %d")
    //   % spec_line_y_pos % bitmap_y_pos % bitmap_y_incr % spec_height << std::endl;
  
    if((x > ll.x) && (x < ur.x) && (y < ll.y) && (y > ur.y)) {
      double fx, fy;
      UnScaleX(wxPoint(x, y), fx);
      // std::cerr << "Got a new frequency selection = " << fx << std::endl;
      radio->SetRXFreqFromDisp(fx); 
    }
    if (refresh_required) Refresh(); 
    event.Skip(); 
  }

  void Waterfall::OnSize(wxSizeEvent & event) {
    ReSize(); 
  }

  void Waterfall::Print(const wxString & fname, const wxString & lnote, const wxString & rnote)
  {
    wxMemoryDC dc;
    width = GetSize().GetWidth();
    height = GetSize().GetHeight();
    wxBitmap pbitmap(width, height);
    dc.SelectObject(pbitmap); 
    DrawOnDC(dc, true, lnote);

    // draw the rightside note --
    wxCoord w, h;
    wxPoint ip;
    ip.x = ur.x;
    ip.y = height - XLABEL_Y_OFFSET;

    wxFont lfont(10, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
    dc.SetFont(lfont);
    dc.GetTextExtent(rnote, &w, &h);
    dc.DrawText(rnote, ip.x - w, ip.y);

    wxImage img = pbitmap.ConvertToImage();
    img.SaveFile(fname, wxBITMAP_TYPE_PNG);
  }

  void Waterfall::Draw(bool redraw_background)
  {
    wxPaintDC dc(this);
    DrawOnDC(dc, redraw_background); 
  }

  void Waterfall::DrawOnDC(wxDC & dc, bool redraw_background, const wxString & note)
  {
    // how big is the window?
    wxSize size = GetSize();
    int width = size.GetWidth();
    int height = size.GetHeight();
    spec_height = height - GRID_BOT_OFFSET;

    if((bitmap != NULL) && ((bitmap->GetHeight() != 2*height) ||
			    (bitmap->GetWidth() != width))) {
      delete bitmap;
      bitmap = NULL; 
    }
			  
    if(redraw_background) {
      dc.Clear(); 
      // Draw the Title and text
      DrawLabels(dc, note);
    }

    if(!enable_draw_events) return; 
    if(!spectrum_data_ready) return;

    if(bitmap == NULL) {
      // create the bitmap that we'll draw on
      bitmap = new wxBitmap(width, 2*height);
      bitmapDC.SelectObject(*bitmap);
      bitmapDC.SetBackground(*wxBLACK_BRUSH);
      bitmapDC.Clear();
      bitmapDC.SetPen(wxColour(0xf0, 0x0f, 0x0f));
      spec_line_y_pos = 0;
      bitmap_y_pos = 0; 
    }
    else {
      bitmapDC.SelectObject(*bitmap);
      // draw the last freq buffer
      DrawSpectrum(bitmapDC); 
      spec_line_y_pos++;
      if(spec_line_y_pos > (spec_height - 1)) {
	spec_line_y_pos = 0;
	bitmap_y_incr = 1; 
      }
    }
  

    // now blit the map onto the display
    int obitpos = spec_line_y_pos;
    dc.Blit(0, 0, width, spec_height, &bitmapDC, 0, obitpos);// bitmap_y_pos);
    bitmap_y_pos += bitmap_y_incr;
    if(bitmap_y_pos > spec_height) bitmap_y_pos = 0; 

    DrawTuningMarkers(dc, 0, spec_height); 

  }

  void Waterfall::DrawSpectrum(wxDC & dc)
  {
    if(!enable_draw_events) return; 
    if(freq_buffer == NULL) return; 
    // find the start and end indices
    double xdelta = freq_buffer[2] - freq_buffer[1];
    double dminidx = floor((xmin - freq_buffer[0]) / xdelta);
    double dmaxidx = floor((xmax - freq_buffer[0]) / xdelta);
    int minidx = ((int) dminidx);
    int maxidx = ((int) dmaxidx);

    if(minidx < 0) minidx = 0;
    if(maxidx >= spectrum_len) maxidx = (spectrum_len - 1); 
  
    float avgpow = AveragePower(minidx, maxidx);
    if(isnan(avgpow)) return ;
    int i, xpos, last_xpos;
    last_xpos = ll.x;
    float cur_max = -200.0;
    for(i = minidx; i <= maxidx; i++) {
      if(ScaleX(freq_buffer[i], xpos)) {
	if(xpos > last_xpos) {
	  // dump the point.
	  dc.SetPen(PowerToHeatColor(cur_max, avgpow));
	  for(int jx = last_xpos + 1; jx <= xpos; jx++) {
	    dc.DrawPoint(jx, spec_line_y_pos); 
	    dc.DrawPoint(jx, spec_line_y_pos + spec_height);
	  }
	  cur_max = power_buffer[i]; 
	}
	else {
	  if(power_buffer[i] > cur_max) cur_max = power_buffer[i]; 	
	}
	last_xpos = xpos; 
      }
    }
  }

  float Waterfall::AveragePower(int minidx, int maxidx)
  {
    float ret = 0.0; 
    int i;
    if(!enable_draw_events) return 0.0; 
    if(freq_buffer == NULL) return 0.0;
    float count = 0.0; 
    for(i = minidx; i <= (maxidx - 6); i += 6) {
      // take the median of 3 -- we'll average those... (!)
      float a, b, c, med;
      a = power_buffer[i];
      b = power_buffer[i+2];
      c = power_buffer[i+4];
      med = ((a > b) ? ((b > c) ? b : c) : ((b > c) ? c : b));
      ret += med;
      count += 1.0; 
    }

    return ret / count; 
  }

  wxColour & Waterfall::PowerToHeatColor(float power, float avg)
  {
    // convert over (heat_range) range. (pow > (avg + (heat_range + heat_floor))) is red
    // (pow < (avg - heat_floor)) is black.  0dB is light blue
    // power arrives in dB relative to something. 
    float frel = ((power - avg) - heat_floor) / heat_range;  // clamp frel to [0,1.0];
    if(frel < 0.0) frel = 0.0;
    if(frel > 1.0) frel = 1.0;

    float fidx = frel * ((float) (HEATMAP_SIZE - 1));
    // std::cerr << boost::format("power = %g  avg = %g  frel = %g  fidx = %g")
    //   % power % avg
    //   % frel % fidx << std::endl; 
    return heatmap[((int) fidx)];
  }

  void Waterfall::InterpolateColor(int idx, wxColour & from, wxColour & to, wxColour & res)
  {
    int r, g, b;
    unsigned char rr, rg, rb;
    float dr = (float) (to.Red() - from.Red());
    float dg = (float) (to.Green() - from.Green());
    float db = (float) (to.Blue() - from.Blue()); 
    float fidx = ((float) idx) / ((float) (HEATMAP_SIZE / 4)); 
    rr = from.Red() + ((int) (fidx * dr)); 
    rg = from.Green() + ((int) (fidx * dg)); 
    rb = from.Blue() + ((int) (fidx * db));

    res.Set(rr, rg, rb);
  }

  void Waterfall::InitHeatMap()
  {
    // simple heat map algorithm from various suggestions on the web
    // for instance, the ruby suggestion at
    // http://stackoverflow.com/questions/12875486/what-is-the-algorithm-to-create-colors-for-a-heatmap
    // is quite interesting.
    // As it is, we'll interpolate between
    // black for 0, light blue for 63, green for 127, yellow for 191, and red for 255


    int of1 = HEATMAP_SIZE / 4;
    int of2 = of1 + of1;
    int of3 = of2 + of1;
    wxColour ltblue(0x0, 0xd0, 0xf5);
    wxColour green(0x0, 0xff, 0x0);
    wxColour yellow(0xff, 0xfc, 0x16);
    wxColour red(0xff, 0x0, 0x0);
    wxColour black(0x0, 0x0, 0x0);
    int i;
    for(i = 0; i < (HEATMAP_SIZE / 4); i++) {
      InterpolateColor(i, black, ltblue, heatmap[i]); 
      InterpolateColor(i, ltblue, green, heatmap[i + of1]); 
      InterpolateColor(i, green, yellow, heatmap[i + of2]); 
      InterpolateColor(i, yellow, red, heatmap[i + of3]); 
    }
  }

  bool Waterfall::UnScaleX(const wxPoint & pt, double & x)
  {
    double ix, iy;
    ix = (double) (pt.x - ll.x);

    x = xmin + ix / x_scale;
  
    return true; 
  }


  bool Waterfall::ScaleX(double x, int & xpos) {
    // fix out-of-range.
    // we have to be really careful about things that are way
    // out of bounds....
    bool ret = true; 
    long lx = ( long) ((x - xmin) * x_scale);
    int ix;
    if((x < xmin) || (x > xmax)) return false;
  
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

    xpos = ll.x + ix; 

    return ret; 
  }



  void Waterfall::DrawData(wxDC & dc)
  {
    // iterate through each trace and draw it.
  
  }

  void Waterfall::DrawLabels(wxDC & dc, const wxString & note)
  {
    wxFont tfont(12, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
    wxFont lfont(10, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);
    wxFont rfont(8, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL);

    dc.SetFont(tfont);
    dc.SetTextForeground(wxColour(0x0, 0x0, 0x0));
    // dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.SetPen(*wxWHITE_PEN);
    dc.SetBrush(*wxWHITE_BRUSH);

    wxPoint cpt;
    wxCoord w, h; 
  
    dc.SetFont(lfont);

    // Draw the center frequency
    double mycf_scalefactor = 1.0e-6;

  
    // Draw the xlabel
    cpt.x = (ll.x + ur.x) / 2;
    cpt.y = height - XLABEL_Y_OFFSET; 
    dc.GetTextExtent(xlabel, &w, &h);
    dc.DrawText(xlabel, cpt.x - w/2, cpt.y);
    dc.DrawText(note, ll.x, cpt.y);
  

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
	tlab.Printf(cf_template, v * mycf_scalefactor);
      }
      else {
	tlab.Printf(x_template, (v - vmid) * x_scalefactor);
      }
      tlab = tlab.Trim(true).Trim(false);
      dc.GetTextExtent(tlab, &w, &h);
      dc.DrawText(tlab, cpt.x - w/2, cpt.y + h/2); 
    }

  }

  void Waterfall::DrawTuningMarkers(wxDC & dc, int yl, int yh)
  {
    // draw the tuning markers
    dc.SetPen(*wxRED);
    dc.SetBrush(*wxBLUE);
    int xl, xh;
    ScaleX(tuning_marker_high, xh); 
    ScaleX(tuning_marker_low, xl); 
    dc.DrawLine(xh, yl, xh, yh);
    dc.DrawLine(xl, yl, xl, yh);
  }

  void Waterfall::ReSize()
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


    x_scale = ((double) graph_width) / (xmax - xmin); // points per hz
    y_scale = ((double) graph_height) / (ymax - ymin); 
    Refresh(); 
  }

  void Waterfall::DrawNew()
  {
    if(freq_buffer == NULL) return;
  
    spectrum_data_ready = true;
    Draw(false); 
  }

  void Waterfall::OnPaint(wxPaintEvent & event)
  {
    Draw(true); 
  }

}

