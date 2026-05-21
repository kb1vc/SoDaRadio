#include "USRPRadio.hxx"

namespace SoDa {
  USRPRadio::USRPRadio(Params * params) : Radio(params) {
    // create the control, rx, and tx  threads. 
    ctrl = USRPCtrl::make(params);
    rx = USRPRX::make(params, ctrl->getUSRP());
    tx = USRPTX::make(params, ctrl->getUSRP());
  }
}

