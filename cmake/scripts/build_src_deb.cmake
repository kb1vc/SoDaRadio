# Runs in cmake script mode (cmake -P) from the package_src_deb custom
# target.  Stages a clean copy of the source tree plus the configured
# debian/ directory, builds the upstream tarball, and invokes
# dpkg-source to produce the .dsc / .debian.tar.xz alongside it.
#
# Inputs (set via -D on the command line):
#   SRC_DIR         source tree root
#   BIN_DIR         build (binary) directory
#   DEBIAN_DIR      configured debian/ directory (with changelog filled in)
#   DEB_SRC_NAME    debian source package name (e.g. sodaradio)
#   VERSION         upstream version (e.g. 11.0.0)

if(NOT SRC_DIR OR NOT BIN_DIR OR NOT DEBIAN_DIR OR NOT DEB_SRC_NAME OR NOT VERSION)
  message(FATAL_ERROR "build_src_deb.cmake: missing required -D arguments")
endif()

set(STAGING "${BIN_DIR}/srcdeb")
set(PKGDIR  "${STAGING}/${DEB_SRC_NAME}-${VERSION}")
set(ORIG_TAR "${STAGING}/${DEB_SRC_NAME}_${VERSION}.orig.tar.xz")

# Wipe any previous staging.
file(REMOVE_RECURSE "${STAGING}")
file(MAKE_DIRECTORY "${PKGDIR}")

# Copy the upstream source tree, excluding build artifacts, VCS metadata,
# editor backups, and any dotfile/dotdir at the source root (which is
# how stray things like .vscode, .claude, .ssh end up in source trees).
file(GLOB _src_entries RELATIVE "${SRC_DIR}" "${SRC_DIR}/*" "${SRC_DIR}/.??*")
foreach(entry IN LISTS _src_entries)
  # Skip dotfiles/dirs at the root.
  if(entry MATCHES "^\\.")
    continue()
  endif()
  # Skip build directories and stray scratch files.
  if(entry MATCHES "^build" OR entry STREQUAL "foo")
    continue()
  endif()
  # Skip editor/backup leftovers and pre-existing tarballs / packages.
  if(entry MATCHES "(~|\\.swp|\\.bak|\\.orig|\\.rej)$")
    continue()
  endif()
  if(entry MATCHES "\\.(tar\\.gz|tar\\.xz|tar\\.bz2|deb|dsc|changes|rpm|buildinfo)$")
    continue()
  endif()
  if(IS_DIRECTORY "${SRC_DIR}/${entry}")
    file(COPY "${SRC_DIR}/${entry}" DESTINATION "${PKGDIR}"
         PATTERN ".git*"   EXCLUDE
         PATTERN ".github" EXCLUDE
         PATTERN ".vscode" EXCLUDE
         PATTERN ".idea"   EXCLUDE
         PATTERN "*~"      EXCLUDE
         PATTERN "*.swp"   EXCLUDE
         PATTERN "*.bak"   EXCLUDE
         PATTERN "*.orig"  EXCLUDE
         PATTERN "*.rej"   EXCLUDE
         PATTERN "foo"     EXCLUDE)
  else()
    file(COPY "${SRC_DIR}/${entry}" DESTINATION "${PKGDIR}")
  endif()
endforeach()

# Build the orig tarball BEFORE we drop in debian/, so the upstream
# tarball contains only the upstream sources (cmake -E tar has no
# --exclude option).
message(STATUS "Creating ${ORIG_TAR}")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar cJf "${ORIG_TAR}"
          "${DEB_SRC_NAME}-${VERSION}"
  WORKING_DIRECTORY "${STAGING}"
  RESULT_VARIABLE _tar_rv)
if(NOT _tar_rv EQUAL 0)
  message(FATAL_ERROR "tar of orig tarball failed (rv=${_tar_rv})")
endif()

# Now drop in the configured debian/ directory.
file(COPY "${DEBIAN_DIR}/" DESTINATION "${PKGDIR}/debian"
     USE_SOURCE_PERMISSIONS)

# Make sure debian/rules is executable -- file(COPY) preserves perms
# but configure_file does not, and we may have come through either.
if(EXISTS "${PKGDIR}/debian/rules")
  execute_process(COMMAND chmod 755 "${PKGDIR}/debian/rules")
endif()

# Build the source package (.dsc + .debian.tar.xz).
message(STATUS "Running dpkg-source -b ${DEB_SRC_NAME}-${VERSION}")
execute_process(
  COMMAND dpkg-source -b "${DEB_SRC_NAME}-${VERSION}"
  WORKING_DIRECTORY "${STAGING}"
  RESULT_VARIABLE _dsc_rv)
if(NOT _dsc_rv EQUAL 0)
  message(FATAL_ERROR
    "dpkg-source failed (rv=${_dsc_rv}). "
    "Install dpkg-dev on this host:  sudo apt install dpkg-dev")
endif()

message(STATUS "Ubuntu source package built in ${STAGING}:")
file(GLOB _outputs
  "${STAGING}/${DEB_SRC_NAME}_${VERSION}*.dsc"
  "${STAGING}/${DEB_SRC_NAME}_${VERSION}*.tar.xz"
  "${STAGING}/${DEB_SRC_NAME}_${VERSION}*.tar.gz")
foreach(f IN LISTS _outputs)
  message(STATUS "  ${f}")
endforeach()
