set(SoDaCLI_SRCS
  SoDaCLI.cxx
  ../src/Command.cxx
  ../src/UDSockets.cxx
)

add_executable(SoDaCLI ${SoDaCLI_SRCS})

target_include_directories(SoDaCLI PRIVATE ../src)

target_link_libraries(SoDaCLI sodautils)

install(TARGETS SoDaCLI DESTINATION bin)
