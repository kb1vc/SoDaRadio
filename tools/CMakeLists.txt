find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(SoDaCreateConfig SoDaCreateConfig.cxx)

target_link_libraries(SoDaCreateConfig
  SoDaUtils::sodautils
  Qt6::Core
)

install(TARGETS SoDaCreateConfig DESTINATION bin)
