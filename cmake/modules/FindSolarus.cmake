# - Find Solarus
# Find the Solarus includes and library.
# Does not look for the Solarus GUI library.
#
#  SOLARUS_INCLUDE_DIRS - where to find solarus headers.
#  SOLARUS_LIBRARIES    - List of libraries when using libsolarus.
#  SOLARUS_FOUND        - True if Solarus was found.
find_path(SOLARUS_INCLUDE_DIR solarus/core/MainLoop.h)

find_library(SOLARUS_LIBRARY solarus)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SOLARUS
    FOUND_VAR SOLARUS_FOUND
    REQUIRED_VARS
        SOLARUS_INCLUDE_DIR
        SOLARUS_LIBRARY)

if(SOLARUS_FOUND)
    set(SOLARUS_INCLUDE_DIRS ${SOLARUS_INCLUDE_DIR})
    set(SOLARUS_LIBRARIES ${SOLARUS_LIBRARY})
endif()
