

set(SimpleSpectrum_SRCS
    SimpleSpectrum_App.cxx
    SimpleSpectrum_GUI.cpp
    SimpleSpectrum_Top.cxx
    SimpleSpectrum_Top_Events.cxx
    SimpleSpectrum_Top_Config.cxx
    RadioThread.cxx
    RadioThread.hxx
    ../gui/waterfall.cxx
    ../gui/waterfall.hxx
    ../gui/FindHome.hxx
    ../gui/FindHome.cxx
    ../src/Debug.hxx
    ../src/Debug.cxx
    )


find_package(wxWidgets COMPONENTS core base richtext aui xrc qa html adv  )
include(${wxWidgets_USE_FILE})
  
add_executable(SimpleSpectrum ${SimpleSpectrum_SRCS})

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/gui)

target_compile_options(SimpleSpectrum BEFORE PUBLIC -Wno-deprecated-declarations)

target_link_libraries(SimpleSpectrum m rt ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

install(TARGETS SimpleSpectrum DESTINATION bin)
