INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_OBROT obrot)

FIND_PATH(
    OBROT_INCLUDE_DIRS
    NAMES obrot/api.h
    HINTS $ENV{OBROT_DIR}/include
        ${PC_OBROT_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    OBROT_LIBRARIES
    NAMES gnuradio-obrot
    HINTS $ENV{OBROT_DIR}/lib
        ${PC_OBROT_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OBROT DEFAULT_MSG OBROT_LIBRARIES OBROT_INCLUDE_DIRS)
MARK_AS_ADVANCED(OBROT_LIBRARIES OBROT_INCLUDE_DIRS)

