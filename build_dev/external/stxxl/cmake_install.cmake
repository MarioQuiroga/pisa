# Install script for directory: /home/mquiroga/tp-final-ir/pisa_dev/external/stxxl

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/mquiroga/tp-final-ir/pisa_dev/external/stxxl/include/stxxl.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/mquiroga/tp-final-ir/pisa_dev/external/stxxl/include/stxxl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/stxxl/bits" TYPE FILE FILES "/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/include/stxxl/bits/config.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/stxxl_debug.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/stxxl" TYPE FILE FILES
    "/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/stxxl-version.cmake"
    "/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/CMakeFiles/stxxl-config.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/stxxl/stxxl-targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/stxxl/stxxl-targets.cmake"
         "/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/CMakeFiles/Export/lib/cmake/stxxl/stxxl-targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/stxxl/stxxl-targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/stxxl/stxxl-targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/stxxl" TYPE FILE FILES "/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/CMakeFiles/Export/lib/cmake/stxxl/stxxl-targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/stxxl" TYPE FILE FILES "/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/CMakeFiles/Export/lib/cmake/stxxl/stxxl-targets-debug.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/lib/cmake_install.cmake")
  include("/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/tools/cmake_install.cmake")
  include("/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/examples/cmake_install.cmake")
  include("/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/tests/cmake_install.cmake")
  include("/home/mquiroga/tp-final-ir/pisa_dev/build_dev/external/stxxl/local/cmake_install.cmake")

endif()

