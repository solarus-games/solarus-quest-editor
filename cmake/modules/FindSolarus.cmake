# - Find Solarus
# Find the Solarus includes and library.
# Does not look for the Solarus GUI library.
#
#  SOLARUS_INCLUDE_DIR - where to find solarus headers.
#  SOLARUS_LIBRARIES   - List of libraries when using libsolarus.
#  SOLARUS_FOUND       - True if Solarus was found.
find_path(SOLARUS_INCLUDE_DIR solarus/core/MainLoop.h)

find_library(SOLARUS_LIBRARY solarus)

# Handle the QUIETLY and REQUIRED arguments and set SOLARUS_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SOLARUS DEFAULT_MSG
  SOLARUS_LIBRARY SOLARUS_INCLUDE_DIR)

if(SOLARUS_FOUND)
  set(SOLARUS_LIBRARIES ${SOLARUS_LIBRARY})
else()
  set(SOLARUS_LIBRARIES)
endif()

mark_as_advanced(SOLARUS_INCLUDE_DIR SOLARUS_LIBRARY)

