#ifndef WXGUI_PARAMS_HDR
#define WXGUI_PARAMS_HDR
#include "../common/GuiParams.hxx"
#include <wx/app.h>

namespace SoDa {
  class WXGuiParams : public SoDa::GuiParams {
  public:
    WXGuiParams(int argc, wxChar ** argv) :
      GuiParams(argc, convertWXargs2Cargs(argc, argv))      
    {
    }
  private:
    // this is really quite gross -- wxApp is not very nice about this. 
    char ** convertWXargs2Cargs(int argc, wxChar ** argv)
    {
      char ** ret;
      ret = new char*[argc];
      int i, j;
      for(i = 0; i < argc; i++) {
	int len;
	for(j = 0; argv[i][j] != (wxChar(0)); j++);
	len = j;
	ret[i] = new char[len+1];
	for(j = 0; j <= len; j++) {
	  ret[i][j] = (char) (argv[i][j] & 0xff); 
	}
      }

      return ret; 
    }
  }; 
}

#endif
