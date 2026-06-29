# SoDaUtils is provided by SoDaLibs (https://github.com/kb1vc/SoDaLibs).
# Install SoDaLibs before building SoDaRadio.
find_package(SoDaUtils REQUIRED CONFIG)

# Verify that both SoDaLibs targets are static — the cmake config package should
# always export them as STATIC_LIBRARY, but fail loudly here if something is wrong.
foreach(_soda_tgt SoDaUtils::sodautils)
  get_target_property(_t "${_soda_tgt}" TYPE)
  if(NOT _t STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR "${_soda_tgt}: expected STATIC_LIBRARY, got ${_t}")
  endif()
endforeach()
