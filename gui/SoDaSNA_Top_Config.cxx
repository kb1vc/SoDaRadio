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
  
  void SoDaSNA_Top::SaveConfig(const wxString & fname)
  {

    config_tree = config_tree_alloc = new boost::property_tree::ptree();
    debugMsg(boost::format("Allocated config tree = %p\n") % config_tree);
  }

  void SoDaSNA_Top::CreateDefaultConfig(boost::property_tree::ptree * config_tree)
  {
    std::stringstream config_stream;

    // write the default config file into the config stream.
#include "DefaultSNA.cfg.h"

    read_xml(config_stream, *config_tree, boost::property_tree::xml_parser::trim_whitespace);    
  }
  

  void SoDaSNA_Top::SetConfigFileName(const wxString & fname) {
    save_config_file_name = fname; 
  }
  
  bool SoDaSNA_Top::LoadConfig(const wxString & fname)
  {
    debugMsg(boost::format("About to delete config tree = %p\n") % config_tree);
    if(config_tree_alloc != NULL) delete config_tree_alloc;

    debugMsg("About to create config tree\n");
	
    config_tree_alloc = new boost::property_tree::ptree();
    debugMsg(boost::format("Got new config tree = %p\n") % config_tree_alloc);
    
    // does the file exist?
    if(!wxFile::Exists(fname.c_str())) {
      // then we need to load the default config.
      debugMsg(boost::format("Creating default config tree = %p\n") % config_tree_alloc);
      CreateDefaultConfig(config_tree_alloc);
      // also pop up the save config dialog box.
      NewConfigDialog * ncd = new NewConfigDialog(this, this);
      ncd->Show();
    }
    else {
      debugMsg(boost::format("Reading config file config tree = %p from %s\n") % config_tree_alloc % fname.c_str());
      std::ifstream config_stream((const char *) fname.mb_str(wxConvUTF8));
      read_xml(config_stream, *config_tree_alloc,
	       boost::property_tree::xml_parser::trim_whitespace);
      config_stream.close();
    }

    debugMsg(boost::format("filled in config tree = %p\n") % config_tree_alloc);
    // The original config format wasn't really proper XML... 
    if(config_tree_alloc->get_child_optional("SoDaRadio")) {
      debugMsg(boost::format("new format config tree = %p\n") % config_tree_alloc);
      config_tree = & config_tree_alloc->get_child("SoDaRadio"); 
    } else {
      debugMsg(boost::format("old format config tree = %p\n") % config_tree_alloc);
      config_tree = config_tree_alloc; 
    }
    
    debugMsg(boost::format("Got the config tree %p\n") % config_tree);
    return true; 
  }

}
