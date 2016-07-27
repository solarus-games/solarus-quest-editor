# - Find Solarus
# Find the Solarus GUI includes and library.
#
#  SOLARUS_GUI_INCLUDE_DIR - where to find solarus headers.
#  SOLARUS_GUI_LIBRARIES   - List of libraries when using libsolarus.
#  SOLARUS_GUI_FOUND       - True if Solarus was found.
find_path(SOLARUS_GUI_INCLUDE_DIR solarus/gui/main_window.h)

find_library(SOLARUS_GUI_LIBRARY solarus-gui)

# Handle the QUIETLY and REQUIRED arguments and set SOLARUS_GUI_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SOLARUS_GUI DEFAULT_MSG
  SOLARUS_GUI_LIBRARY SOLARUS_GUI_INCLUDE_DIR)

if(SOLARUS_GUI_FOUND)
  set(SOLARUS_GUI_LIBRARIES ${SOLARUS_GUI_LIBRARY})
else()
  set(SOLARUS_GUI_LIBRARIES)
endif()

mark_as_advanced(SOLARUS_GUI_INCLUDE_DIR SOLARUS_GUI_LIBRARY)

