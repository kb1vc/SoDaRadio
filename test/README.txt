For build integrity tests.

build an apptainer like this

  sudo apptainer build  Ubuntu20r04.sif Ubuntu20r04.def 

This will build the radio

Now

  apptainer shell Ubuntu20r04.sif

and then try

  /SoDaRadio/build/qtgui/SoDaRadio --server /SoDaRadio/build/src/SoDaServer

and watch what happens!

