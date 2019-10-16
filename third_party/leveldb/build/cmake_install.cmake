# Install script for directory: /mnt/d/Codes/cs754-nfs/third_party/leveldb

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
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/mnt/d/Codes/cs754-nfs/third_party/leveldb/build/libleveldb.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/leveldb" TYPE FILE FILES
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/c.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/cache.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/comparator.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/db.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/dumpfile.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/env.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/export.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/filter_policy.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/iterator.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/options.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/slice.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/status.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/table_builder.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/table.h"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/include/leveldb/write_batch.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets.cmake"
         "/mnt/d/Codes/cs754-nfs/third_party/leveldb/build/CMakeFiles/Export/lib/cmake/leveldb/leveldbTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb/leveldbTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb" TYPE FILE FILES "/mnt/d/Codes/cs754-nfs/third_party/leveldb/build/CMakeFiles/Export/lib/cmake/leveldb/leveldbTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb" TYPE FILE FILES "/mnt/d/Codes/cs754-nfs/third_party/leveldb/build/CMakeFiles/Export/lib/cmake/leveldb/leveldbTargets-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/leveldb" TYPE FILE FILES
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/cmake/leveldbConfig.cmake"
    "/mnt/d/Codes/cs754-nfs/third_party/leveldb/build/leveldbConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/mnt/d/Codes/cs754-nfs/third_party/leveldb/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
