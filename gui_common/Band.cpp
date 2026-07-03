/*
Copyright (c) 2017,2025 Matthew H. Reilly (kb1vc)
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



#include <QString>
#include <QObject>
#include <QSettings>
#include <QMap>

#include "Band.hpp"

namespace GUISoDa {

  Band::Band() {
      band_name = ""; 
      band_index = 0; 
      def_rx_ant = "";
      def_tx_ant = "";
      min_freq = 0.0; 
      max_freq = 0.0; 
      def_mode = "";
      tx_enabled = false; 
      tverter_mode = false; 
      lowside_injection = false; 
      tv_LO_freq = 0.0; 
      tv_LO_mult = 0.0;
      last_rx_freq = 0.0;
      last_tx_freq = 0.0;

      sat_offset = 0.0; 
      full_duplex = false; 
      sat_offset_enabled = false; 
    }

  void Band::restore(QSettings * set_p) {
      band_name = set_p->value("Name").toString();
      band_index = set_p->value("Index").toInt();

      def_rx_ant = set_p->value("DefRXAnt").toString();
      def_tx_ant = set_p->value("DefTXAnt").toString();

      def_mode = set_p->value("DefMode").toString();    

      min_freq = set_p->value("MinFreq").toDouble();
      max_freq = set_p->value("MaxFreq").toDouble();

      last_rx_freq = set_p->value("LastRXFreq").toDouble();
      last_tx_freq = set_p->value("LastTXFreq").toDouble();

      tx_enabled = set_p->value("TXEna").toBool();    

      tv_LO_freq = set_p->value("TVLOFreq").toDouble();
      tv_LO_mult = set_p->value("TVLOMult").toDouble();
      tverter_mode = set_p->value("TVMode").toBool();
      lowside_injection = set_p->value("TVLowInjection").toBool();    

      // satellite mode. 
      sat_offset = set_p->value("SatOffset", 0.0).toDouble();
      full_duplex = set_p->value("FullDuplex", false).toBool();
      sat_offset_enabled = set_p->value("SatOffsetEna", false).toBool(); 
    }

  void Band::save(QSettings * set_p) const {
      set_p->setValue("Name", band_name);
      set_p->setValue("Index", band_index);
      set_p->setValue("DefRXAnt", def_rx_ant);
      set_p->setValue("DefTXAnt", def_tx_ant);
      set_p->setValue("DefMode", def_mode);    
      set_p->setValue("MinFreq", min_freq);
      set_p->setValue("MaxFreq", max_freq);
      set_p->setValue("LastRXFreq", last_rx_freq);
      set_p->setValue("LastTXFreq", last_tx_freq);
      set_p->setValue("TVLOFreq", tv_LO_freq);
      set_p->setValue("TVLOMult", tv_LO_mult);
      set_p->setValue("TXEna", tx_enabled);
      set_p->setValue("TVMode", tverter_mode);
      set_p->setValue("TVLowInjection", lowside_injection);

      set_p->setValue("SatOffset", sat_offset);
      set_p->setValue("FullDuplex", full_duplex);
      set_p->setValue("SatOffsetEna", sat_offset_enabled); 

    }



  bool Band::isInBand(double freq) const {
      double tf = freq * 1e-6; 
      bool res = (tf >= min_freq) && (tf <= max_freq); 
      return res; 
    }
}

