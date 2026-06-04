set(SoDaCLI_SRCS
  SoDaCLI.cxx
  CLICommand.cxx
  CLIAudio.cxx
  ../src/Command.cxx
  ../src/UDSockets.cxx
)

find_package(Qt6 REQUIRED COMPONENTS Core Multimedia Network)

add_executable(SoDaCLI ${SoDaCLI_SRCS})

set_target_properties(SoDaCLI PROPERTIES AUTOMOC ON)

target_include_directories(SoDaCLI PRIVATE ../src)

target_link_libraries(SoDaCLI
  sodautils
  Qt6::Core
  Qt6::Multimedia
  Qt6::Network
)

install(TARGETS SoDaCLI DESTINATION bin)
