
########### next target ###############

set(FORMAT_SRCS
	Format.cxx
	)

add_library(sodaformat SHARED ${FORMAT_SRCS})
install(TARGETS sodaformat DESTINATION lib)

# Export header files that are required to build loadable modules

install(FILES Format.hxx DESTINATION "include/SoDaFormat")

add_executable(Format_Test Format_Test.cxx)
target_link_libraries(Format_Test sodaformat)


