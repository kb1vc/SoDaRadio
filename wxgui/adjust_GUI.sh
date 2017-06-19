#!/bin/bash

# adjust_GUI.sh source_dir working_dir  dest_dir 
#  adjust  the wxformbuilder output cpp file to 
# use xpm icons for the tuning buttons.
source_dir=$1
working_dir=$2
dest_dir=$3

echo "#include \"uparrow.xpm\"" > ${working_dir}/temporaryjunk.cpp
echo "#include \"downarrow.xpm\"" >> ${working_dir}/temporaryjunk.cpp
echo "#include \"SoDaLogo_Big.xpm\"" >> ${working_dir}/temporaryjunk.cpp

cat ${working_dir}/temporaryjunk.cpp ${source_dir}/SoDaRadio_GUI.cpp > ${working_dir}/temporaryjunk2.cpp

sed -e 's/wxT("downarrow.png"), wxBITMAP_TYPE_ANY/downarrow_xpm/g' < ${working_dir}/temporaryjunk2.cpp |  \
  sed -e 's/wxT("SoDaLogo_Big.png"), wxBITMAP_TYPE_ANY/SoDaLogo_Big_xpm/g' | \
  sed -e 's/wxT("uparrow.png"), wxBITMAP_TYPE_ANY/uparrow_xpm/g' > ${dest_dir}/SoDaRadio_GUI.cxx

rm ${working_dir}/temporaryjunk.cpp ${working_dir}/temporaryjunk2.cpp

