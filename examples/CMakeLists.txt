install(FILES example_CMakeLists.txt DESTINATION "share/SoDaRadio/examples" RENAME CMakeLists.txt)

set(OtherExampleFiles
  IFServer.cxx
  IFServer.hxx
  IFServer_Test.cxx)

install(FILES ${OtherExampleFiles} DESTINATION "share/SoDaRadio/examples")
