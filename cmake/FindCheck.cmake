find_path(CHECK_INCLUDE_DIR
        NAMES check.h
        PATH_SUFFIXES check
        DOC "Path to the Check headers"
)

find_library(CHECK_LIBRARIES
        NAMES check
        DOC "Check testing library"
)

# Mark as found if both include directory and library are found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Check DEFAULT_MSG CHECK_LIBRARIES CHECK_INCLUDE_DIR)

# If found, set variables for use in the main CMakeLists.txt
if(CHECK_FOUND)
    set(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIR})
    set(CHECK_LIBRARIES ${CHECK_LIBRARIES})
endif()