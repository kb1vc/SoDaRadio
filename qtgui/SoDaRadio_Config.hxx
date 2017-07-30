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

#ifndef SODARADIO_CONFIG_HDR
#define SODARADIO_CONFIG_HDR
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../src/Debug.hxx"

class SoDaRadio_Config : public SoDa::Debug {
public:
  SoDaRadio_Config(const std::string & config_fname) :
    SoDa::Debug("SoDaRadio_Config") {
    // read the config tree
    std::ifstream cnf_in(config_fname.c_str());

    // if it is not present, load a default tree
    if(cnf_in.is_open()) {
      read_xml(cnf_in, &configuration_tree, 
	       boost::property_tree::xml_parser::trim_whitespace);
    }
    else {
      loadDefaultConfig();
    }
  }

  explicit SoDaRadio_Config() :
    SoDa::Debug("SoDaRadio_Config") {
    // no config tree present.  Load a default 2m configuration
    
    loadDefaultConfig();
  }

  void loadDefaultConfig(); 

  void saveConfig(const std::string & config_fname); 
  void connect(

protected:
  boost::property_tree::ptree configuration_tree; 
}; 
#endif

