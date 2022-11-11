/*
  Copyright (c) 2017, 2022 Matthew H. Reilly (kb1vc)
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

#include <string>
#include <iostream>
#include "USRPFrontEnd.hxx"
#include "USRPPropTree.hxx"

namespace SoDa {
  USRPPropTree * getUSRPFrontEnd(USRPPropTree * tree, char tr_choice) {
    // find a daughterboard.
    std::vector<std::string> dblist = tree->getPropNames("dboards");
    
    std::string fe_name;
    switch(tr_choice) {
    case 'r': 
    case 'R': 
      fe_name = "/rx_frontends";
      break; 
    case 't':
    case 'T':
      fe_name = "/tx_frontends";
      break; 
    default:
      return NULL; 
    }

    for(auto dbn: dblist) {
      // now go down the list of tx/rx front ends
      std::string fe_listn = "dboards/" + dbn + fe_name; 
      std::vector<std::string> felist = tree->getPropNames(fe_listn);
      for(auto fen: felist) {
	fe_name = fe_listn + "/" + fen; 
	// now see what the connection type is.... 
	std::string conn_type = tree->getStringProp(fe_name + "/connection");
	if((conn_type == "IQ") || (conn_type == "QI")) {
	  return new USRPPropTree(tree, fe_name);
	}
      }
    }

    return NULL; 
  }

  USRPPropTree * getUSRPFrontEnd(USRPPropTree & tree, char tr_choice) {
    return getUSRPFrontEnd(&tree, tr_choice);
  }  
}
