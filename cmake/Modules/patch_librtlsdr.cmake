# cmake/Modules/patch_librtlsdr.cmake
#
# Standalone script invoked from BuildLibRTLSDR.cmake's PATCH_COMMAND as
#   cmake -DFILE=<path-to-librtlsdr-CMakeLists.txt> -P patch_librtlsdr.cmake
#
# librtlsdr's CMakeLists.txt uses ${CMAKE_SOURCE_DIR} in a handful of
# places (module path, include dir, uninstall configure_file) where it
# should be ${CMAKE_CURRENT_SOURCE_DIR}.  Under FetchContent the former
# resolves to SoDaRadio's source tree instead of librtlsdr's, so the
# build breaks.  Rewrite the file in place.

file(READ "${FILE}" _content)
string(REPLACE
  [=[${CMAKE_SOURCE_DIR}]=]
  [=[${CMAKE_CURRENT_SOURCE_DIR}]=]
  _content "${_content}")

# librtlsdr adds a custom "uninstall" target whose name collides with the
# same-named target libiio creates.  In a superbuild where both are fetched,
# only one can win.  Rename librtlsdr's out of the way -- SoDaRadio doesn't
# use it, and neither does anything else in the sub-tree.
string(REPLACE
  "add_custom_target(uninstall"
  "add_custom_target(librtlsdr_uninstall"
  _content "${_content}")

file(WRITE "${FILE}" "${_content}")
