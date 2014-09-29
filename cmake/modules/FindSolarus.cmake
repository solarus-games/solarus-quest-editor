# - Find Solarus
# Find the Solarus includes and library.
#
#  SOLARUS_INCLUDE_DIR - where to find solarus headers.
#  SOLARUS_LIBRARIES   - List of libraries when using libsolarus.
#  SOLARUS_FOUND       - True if Solarus was found.

if(SOLARUS_INCLUDE_DIR)
  # Already in cache, be silent
  set(SOLARUS_FIND_QUIETLY TRUE)
endif(SOLARUS_INCLUDE_DIR)

find_path(SOLARUS_INCLUDE_DIR solarus/MainLoop.h)

find_library(SOLARUS_LIBRARY NAMES solarus)

# Handle the QUIETLY and REQUIRED arguments and set SOLARUS_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SOLARUS DEFAULT_MSG
  SOLARUS_INCLUDE_DIR SOLARUS_LIBRARY)

if(SOLARUS_FOUND)
  set(SOLARUS_LIBRARIES ${SOLARUS_LIBRARY})
else(SOLARUS_FOUND)
  set(SOLARUS_LIBRARIES)
endif(SOLARUS_FOUND)

mark_as_advanced(SOLARUS_INCLUDE_DIR SOLARUS_LIBRARY)

