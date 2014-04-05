/*
  Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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
#include "SoDaRadio_App.hxx"
#include "SoDaRadio_Top.h"
#include "GuiParams.hxx"


IMPLEMENT_APP(SoDaRadio_App)

SoDaRadio_App::SoDaRadio_App()
{
}

SoDaRadio_App::~SoDaRadio_App()
{
}

bool SoDaRadio_App::OnInit()
{
  // get the args and argvs
  SoDa::GuiParams  p(argc, argv);
  std::cerr << "got params." << std::endl;
  SoDaRadio_GUI::SoDaRadio_Top * top = new SoDaRadio_GUI::SoDaRadio_Top(p, (wxWindow*) NULL);

  std::cerr << "created top." << std::endl; 
  top->Show();
  std::cerr << "showed top." << std::endl; 
  SetTopWindow(top);
  std::cerr << "set top." << std::endl; 
  wxInitAllImageHandlers();
  std::cerr << "inited headers." << std::endl;
  return true; 
}
