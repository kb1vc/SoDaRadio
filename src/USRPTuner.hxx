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

 ///
 ///  @file USRPTuner.hxx
 ///  @brief Class that encapsulates tuning functions for various daughter cards
 ///
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   November 2015
 ///

#ifndef USRPTUNER_HDR
#define USRPTUNER_HDR
#include "SoDaBase.hxx"
#include "Params.hxx"


#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/dboard_base.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/types/tune_result.hpp>
#include <uhd/utils/msg.hpp>

namespace SoDa {

  /// @class USRPTuner
  /// 
  /// Generic class for TX and RX tuning.  This is specialized for
  /// each of the daughtercard modules that needs specialization. 
  /// The base class implements the standard tuning interface
  /// (typically fractional-N). 
  /// 
  /// 
  class USRPTuner : public Debug {
  public:
    /// @brief Constructor -- 
    /// @param unit_name the name of this tuner
    /// @param usrp a pointer to the USRP object that owns this tuner
    /// @param min_separation where possible, the LO frequency should 
    /// be at least min_separation Hz from a specified "avoid_frequency."
    USRPTuner(uhd::usrp::multi_usrp::sptr usrp, 
	      double min_separation, 
	      std::string unit_name = std::string("USRPTuner")); 


    /// @brief Factory to build a particular subclass of USRPTuner based on 
    /// the detected front-end module. 
    /// @param usrp a pointer to the USRP object that owns this tuner
    /// @param min_separation where possible, the LO frequency should 
    /// be at least min_separation Hz from a specified "avoid_frequency."
    /// @param force_simple if true, turn off intN tuning mode even if it supported, 
    /// use the simplest version of the tuner (

    USRPTuner * makeTuner(uhd::usrp::multi_usrp::sptr usrp, 
			  double min_separation, 
			  bool force_fracN = false, 
			  bool force_simple = false);

    /// @brief setRXFreq set the RX 1st LO frequency.  Where possible,
    /// the chosen frequency should be separated from the avoid_freq
    /// by the amount specified when we created this tuning object. 
    ///
    /// @param rx_freq the target frequency of interest
    /// @param avoid_freq the frequency that we must avoid by at least min_sep
    /// @param tune_result the response from the UHD set_xx_freq request
    /// @return true on success, false on failure. 
    ///
    virtual bool setRXFreq(double rx_freq, double avoid_freq, uhd::tune_result_t & tune_result) = 0;
    /// @brief setTXFreq set the TX 1st LO frequency.  Where possible,
    /// the chosen frequency should be separated from the avoid_freq
    /// by the amount specified when we created this tuning object. 
    ///
    /// @param tx_freq the target frequency of interest
    /// @param avoid_freq the frequency that we must avoid by at least min_sep
    /// @param tune_result the response from the UHD set_xx_freq request
    /// @return true on success, false on failure. 
    ///
    virtual bool setTXFreq(double tx_freq, double avoid_freq, uhd::tune_result_t & tune_result) = 0;

  protected:
    /// is the identified (rx or tx) front-end LO locked?
    /// If not, set the tuning frequency to "the right thing"
    /// @param req the requested frequency (and tuning discipline)
    /// @param sel 'r' for RX LO, 't' for TX LO
    /// @param cur tuning result, if the LO was locked.
        /// @return true if the LO is locked, false otherwise. 
    virtual bool checkLock(uhd::tune_request_t & req,
			   char sel,
			   uhd::tune_result_t & cur);

    double min_separation;
    uhd::usrp::multi_usrp::sptr usrp;
    bool has_lock_detect; 
  }; 

  /// 
  /// @class IntNTuner
  ///
  /// This is a tuner for the UBX, WBX, or SBX module that takes advantage of 
  /// the int_n_step argument in the tune request. 
  class IntNTuner : public USRPTuner {
  public:
    /// @brief Constructor -- 
    /// @param unit_name the name of this tuner
    /// @param usrp a pointer to the USRP object that owns this tuner
    /// @param min_separation where possible, the LO frequency should 
    /// be at least min_separation Hz from a specified "avoid_frequency."
    IntNTuner(uhd::usrp::multi_usrp::sptr usrp, 
	      double min_separation) : 
      USRPTuner(usrp, min_separation, std::string("IntNTuner")) 
    {
      initTuner(); 
    }

    /// @brief setRXFreq set the RX 1st LO frequency.  Where possible,
    /// the chosen frequency should be separated from the avoid_freq
    /// by the amount specified when we created this tuning object. 
    ///
    /// @param rx_freq the target frequency of interest
    /// @param avoid_freq the frequency that we must avoid by at least min_sep
    /// @param tune_result the response from the UHD set_xx_freq request
    /// @return true on success, false on failure. 
    ///
    bool setRXFreq(double rx_freq, double avoid_freq, uhd::tune_result_t & tune_result);

    /// @brief setTXFreq set the TX 1st LO frequency.  Where possible,
    /// the chosen frequency should be separated from the avoid_freq
    /// by the amount specified when we created this tuning object. 
    ///
    /// @param tx_freq the target frequency of interest
    /// @param avoid_freq the frequency that we must avoid by at least min_sep
    /// @param tune_result the response from the UHD set_xx_freq request
    /// @return true on success, false on failure. 
    ///
    virtual bool setTXFreq(double tx_freq, double avoid_freq, uhd::tune_result_t & tune_result);

  private:
    void initTuner();
  };

  /// 
  /// @class SimpleTuner
  ///
  /// This is a simple front end tuner.  It should work with all 
  /// front-ends and configurations, even the LFTX/LFRX and BASICTX/RX
  /// which don't have front-end oscillators at all. 
  /// WBX or SBX modules
  class SimpleTuner : public USRPTuner {
  public:
    /// @brief Constructor -- 
    /// @param unit_name the name of this tuner
    /// @param usrp a pointer to the USRP object that owns this tuner
    SimpleTuner(uhd::usrp::multi_usrp::sptr usrp) : 
      USRPTuner(usrp, 0.0, std::string("IntNTuner")) 
    {
      initTuner(); 
    }

    /// @brief setRXFreq set the RX 1st LO frequency.  Where possible,
    /// the chosen frequency should be separated from the avoid_freq
    /// by the amount specified when we created this tuning object. 
    ///
    /// @param rx_freq the target frequency of interest
    /// @param avoid_freq the frequency that we must avoid by at least min_sep (ignored)
    /// @param tune_result the response from the UHD set_xx_freq request
    /// @return true on success, false on failure. 
    ///
    bool setRXFreq(double rx_freq, double avoid_freq, uhd::tune_result_t & tune_result)
    {
      uhd::tune_request_t rx_req(rx_freq); 
      tune_result = usrp->set_rx_freq(rx_req); 
      // no need to check lock. 
      return true; 
    }

    /// @brief setTXFreq set the TX 1st LO frequency.  Where possible,
    /// the chosen frequency should be separated from the avoid_freq
    /// by the amount specified when we created this tuning object. 
    ///
    /// @param tx_freq the target frequency of interest
    /// @param avoid_freq the frequency that we must avoid by at least min_sep (ignored)
    /// @param tune_result the response from the UHD set_xx_freq request
    /// @return true on success, false on failure. 
    ///
    bool setTXFreq(double tx_freq, double avoid_freq, uhd::tune_result_t & tune_result)
    {
      uhd::tune_request_t tx_req(tx_freq); 
      tune_result = usrp->set_tx_freq(tx_req); 
      // no need to check lock. 
      return true; 
    }

  private:
    void initTuner() {
      // do nothing. 
    }
  };

}
#endif
