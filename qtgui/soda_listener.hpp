/*
Copyright (c) 2017 Matthew H. Reilly (kb1vc)
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

#ifndef SODA_LISTENER_HEADER
#define SODA_LISTENER_HEADER
#include <QObject>
#include <QString>
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <errno.h>
#include "../src/Command.hxx"

namespace GUISoDa {
  
  class Listener : public QObject {
    Q_OBJECT

  public:
    Listener(QObject * parent = 0, const QString & socket_basename = "tmp");
    ~Listener() {
    }

    /**
     * @brief connect to radio server sockets and initialize listener state
     * 
     * @return true on success, false on some fatal problem. 
     */
    bool init();   

  
    /**
     * @brief initiate transfers on the socket.
     */
    void start();

  
  signals:
    // when new spectrum data arrives
    void updateData(double cfreq, float * y);

    void configureSpectrum(double cfreq, double span, long buckets);
  
    void addModulation(QString modtype, int mod_id);
    void addFilterWidth(double lo, double hi);
    void addFilterName(QString filter_name, int filt_id);
    void repMarkerOffset(double lo, double hi);
  
    void addRXAntName(const QString & ant_name);
    void addTXAntName(const QString & ant_name);  

    void repGainRange(double min_power, double max_power);

    void repFilterEdges(double lo, double hi);

    void repGPSLatLon(double lat, double lon);
    void repGPSTime(int hh, int mm, int ss);

    void repGPSLock(bool is_locked);

    void repSDRVersion(const QString & version);
    void repHWMBVersion(const QString & version);

    void repPTT(bool on); 

    void initSetupComplete();

    void fatalError(const QString & error_string);
					      
  public slots:
    void setRXFreq(double freq);
    void setTXFreq(double freq);

    void setModulation(int mod_id); 

    void setAFFilter(int id); 

    void setRXGain(int gain);
    void setTXGain(int gain);
    void setAFGain(int gain);
    void setAFSidetoneGain(int gain);  

    void setRXAnt(const QString & antname);
    void setTXAnt(const QString & antname);

    double getRXFreq() { return current_rx_freq; }
    double getTXFreq() { return current_tx_freq; }

    void setSpectrumCenter(double freq); 
    void setSpectrumAvgWindow(int window);
    void setSpectrumUpdateRate(int rate);  

    void setCWSpeed(int speed); 
    void setSidetoneVolume(int vol);

    void setSquelchLevel(int lev);

    void setClockRef(int external);

    void setPTT(bool on, bool full_duplex = false);
    void setCarrier(bool on);    

    void recordRF(int checkbox_state);
    
    void sendCW(const QString & txt);
    void clearCWBuffer();
  
    void closeRadio();


  protected:
    double current_rx_freq; 
    double current_tx_freq; 
    int get(char* buf, int maxlen); 
    bool get(SoDa::Command & cmd); 
    int put(const char * buf, int len);
    bool put(const SoDa::Command & cmd, const char * func_name = "?");
    

    bool handleREP(const SoDa::Command & cmd);
    bool handleSET(const SoDa::Command & cmd);
    bool handleGET(const SoDa::Command & cmd); 

    /// gain range from min to max (reported by radio)
    double tx_gain_min;
    double tx_gain_max;
    
  private:
    void setupSpectrumBuffer(double cfreq, double span, long buflen);
    long spect_buffer_len;
    float * spect_buffer; 		      
    double spect_center_freq; 				    
			  
  protected slots:  
    void processCmd();
    void cmdErrorHandler(QLocalSocket::LocalSocketError err) {
      std::cerr << "Error [" << err << "]\n";
    }

    void processSpectrum();

  private:
    QString socket_basename; 
    QLocalSocket * cmd_socket;
    QLocalSocket * spect_socket;
    bool quit; 
  };
}
#endif
