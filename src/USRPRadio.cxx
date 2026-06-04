#include "USRPRadio.hxx"

namespace SoDa {
  USRPRadio::USRPRadio(ParamsPtr params) : Radio(params) {
    // create the control, rx, and tx  threads. 
    ctrl = USRPCtrl::make(params);
    rx = USRPRX::make(params, ctrl->getUSRP());
    tx = USRPTX::make(params, ctrl->getUSRP());
  }
  
  void USRPRadio::init() {
    // we don't really need to do anything here.
    ctrl->init();
    rx->init();
    tx->init();
  }
}

