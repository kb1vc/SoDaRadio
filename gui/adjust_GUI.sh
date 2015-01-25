#!/bin/bash

#  adjust  the wxformbuilder output cpp file to 
# use xpm icons for the tuning buttons. 

echo "#include \"uparrow.xpm\"" > temporary_junk.cpp
echo "#include \"downarrow.xpm\"" >> temporary_junk.cpp
echo "#include \"SoDaLogo_Big.xpm\"" >> temporary_junk.cpp

cat temporary_junk.cpp ../../gui/SoDaRadio_GUI.cpp > temporary_junk2.cpp

sed -e 's/wxT("downarrow.png"), wxBITMAP_TYPE_ANY/downarrow_xpm/g' < temporary_junk2.cpp |  \
  sed -e 's/wxT("SoDaLogo_Big.png"), wxBITMAP_TYPE_ANY/SoDaLogo_Big_xpm/g' | \
  sed -e 's/wxT("uparrow.png"), wxBITMAP_TYPE_ANY/uparrow_xpm/g' > SoDaRadio_GUI.cxx

rm temporary_junk.cpp temporary_junk2.cpp


echo "#include \"SoDaLogo_Big.xpm\"" >> temporary_junk.cpp

cat temporary_junk.cpp ../../gui/SoDaSNA_GUI.cpp > temporary_junk2.cpp

sed -e 's/wxT("SoDaLogo_Big.png"), wxBITMAP_TYPE_ANY/SoDaLogo_Big_xpm/g' < temporary_junk2.cpp  > SoDaSNA_GUI.cxx

rm temporary_junk.cpp temporary_junk2.cpp
