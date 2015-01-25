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

#include "SoDaSNA_Top.h"
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/colour.h>
#include "../src/Command.hxx"
#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>

using namespace std; 
namespace SoDaSNA_GUI {

  void SoDaSNA_Top::StartPassThroughCalibration(CalibrateDialog * dlg) {
    dlg->AppendProgressMsg(wxT("Starting pass-through calibration\n"));
    measurement_mode = CAL_PASS; 
    CompletePassThroughCalibration(dlg);
    
  }

  void SoDaSNA_Top::StartOpenCalibration(CalibrateDialog * dlg) {
    dlg->AppendProgressMsg(wxT("Starting open-circuit calibration\n"));
    measurement_mode = CAL_OPEN;     
    CompleteOpenCalibration(dlg);    
  }

  void SoDaSNA_Top::CompletePassThroughCalibration(CalibrateDialog * dlg) {
    wxThread::Sleep(10000);    
    dlg->AppendProgressMsg(wxT("Completed pass-through calibration\n"));
    measurement_mode = IGNORE;             
    dlg->CompleteStep();
  }

  void SoDaSNA_Top::CompleteOpenCalibration(CalibrateDialog * dlg) {
    wxThread::Sleep(10000);
    dlg->AppendProgressMsg(wxT("Completed open-circuit calibration\n"));
    measurement_mode = IGNORE;
    dlg->CompleteStep();    
  }
  
}
