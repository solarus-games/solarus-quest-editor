# - Find Solarus
# Find the Solarus GUI includes and library.
#
#  SOLARUS_GUI_INCLUDE_DIRS - where to find solarus GUI headers.
#  SOLARUS_GUI_LIBRARIES    - List of libraries when using libsolarus-gui.
#  SOLARUS_GUI_FOUND        - True if Solarus GUI was found.
find_path(SOLARUS_GUI_INCLUDE_DIR solarus/gui/main_window.h)

find_library(SOLARUS_GUI_LIBRARY solarus-gui)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SOLARUS_GUI
    FOUND_VAR SOLARUS_GUI_FOUND
    REQUIRED_VARS
        SOLARUS_GUI_INCLUDE_DIR
        SOLARUS_GUI_LIBRARY)

if(SOLARUS_GUI_FOUND)
    set(SOLARUS_GUI_INCLUDE_DIRS ${SOLARUS_GUI_INCLUDE_DIR})
    set(SOLARUS_GUI_LIBRARIES ${SOLARUS_GUI_LIBRARY})
endif()
