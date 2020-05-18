# CMake is so ugly!
cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME "jimtclpp")
project(${PROJ_NAME} VERSION 1.0.0 LANGUAGES CXX)

# Add option to build in Debug or Release version
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS  ${CMAKE_CONFIGURATION_TYPES})

# Add postfix to debug output
SET(CMAKE_DEBUG_POSTFIX "d")

# Where to place output
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}) 

# Change include path
include_directories(${PROJECT_SOURCE_DIR}/inc)
include_directories(${PROJECT_SOURCE_DIR}/priv_inc)
include_directories(${PROJECT_SOURCE_DIR}/versions)

# Add options to compiler
# set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER g++)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_UTF8=1")
set(BUILDDIR "")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message("GNU Compiler")  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-result")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message("MSVC Compiler")
  set(BUILDIR "${CMAKE_BUILD_TYPE}/")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()
  message("UKNOWN Compiler ${CMAKE_CXX_COMPILER_ID}")
endif()

# Various versions of C++ standard to test against.
# set(CMAKE_CXX_STD 11)
# set(CMAKE_CXX_STD 14)
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

# Build static library jimpp
add_library(jimpp STATIC
${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp

${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-sockets.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-ssl.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-history-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-interp-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-load-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-namespace-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-pack-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-package-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-readdir-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-tclprefix-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-posix-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-signal-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-syslog-ext.cpp
${CMAKE_SOURCE_DIR}/core/jim-format.cpp
${CMAKE_SOURCE_DIR}/core/jim-interactive.cpp
${CMAKE_SOURCE_DIR}/core/jim-subcmd.cpp
${CMAKE_SOURCE_DIR}/core/jim.cpp
${CMAKE_SOURCE_DIR}/core/jim-hashtable.cpp
${CMAKE_SOURCE_DIR}/core/utf8.cpp
${CMAKE_SOURCE_DIR}/core/_load-static-exts.cpp
${CMAKE_SOURCE_DIR}/core/prj_trace.cpp
${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp
${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tree-sext.cpp
)

# Code to build extensions static and dynamic libraries
# include("extensions.cmake")

# Build shared library jimppso - disabled for now.
# add_library(jimppso SHARED
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp
# 
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-sockets.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-ssl.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-history-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-interp-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-load-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-namespace-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-pack-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-package-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-readdir-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-tclprefix-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-posix-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-signal-ext.cpp
# ${CMAKE_SOURCE_DIR}/binary_ext/jim-syslog-ext.cpp
# ${CMAKE_SOURCE_DIR}/core/jim-format.cpp
# ${CMAKE_SOURCE_DIR}/core/jim-interactive.cpp
# ${CMAKE_SOURCE_DIR}/core/jim-subcmd.cpp
# ${CMAKE_SOURCE_DIR}/core/jim.cpp
# ${CMAKE_SOURCE_DIR}/core/jim-hashtable.cpp
# ${CMAKE_SOURCE_DIR}/core/utf8.cpp
# ${CMAKE_SOURCE_DIR}/core/_load-static-exts.cpp
# ${CMAKE_SOURCE_DIR}/core/prj_trace.cpp
# ${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
# ${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp
# ${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
# ${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
# ${CMAKE_SOURCE_DIR}/script_ext/_tree-sext.cpp
# )

# Build executable jimsh
add_executable(jimshpp 
${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp

${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-sockets.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-ssl.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-history-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-interp-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-load-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-namespace-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-pack-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-package-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-readdir-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-tclprefix-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-posix-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-signal-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-syslog-ext.cpp
${CMAKE_SOURCE_DIR}/core/jim-format.cpp
${CMAKE_SOURCE_DIR}/core/jim-interactive.cpp
${CMAKE_SOURCE_DIR}/core/jim-subcmd.cpp
${CMAKE_SOURCE_DIR}/core/jim.cpp
${CMAKE_SOURCE_DIR}/core/jim-hashtable.cpp
${CMAKE_SOURCE_DIR}/core/utf8.cpp
${CMAKE_SOURCE_DIR}/core/_load-static-exts.cpp
${CMAKE_SOURCE_DIR}/core/prj_trace.cpp
${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp
${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tree-sext.cpp

${CMAKE_SOURCE_DIR}/jimsh/jimsh.cpp
)

# Smallest possible jimsh
add_executable(jimshppmin 
  ${CMAKE_SOURCE_DIR}/core/jim-format.cpp
  ${CMAKE_SOURCE_DIR}/core/jim-interactive.cpp
  ${CMAKE_SOURCE_DIR}/core/jim-subcmd.cpp
  ${CMAKE_SOURCE_DIR}/core/jim.cpp
  ${CMAKE_SOURCE_DIR}/core/jim-hashtable.cpp
  ${CMAKE_SOURCE_DIR}/core/utf8.cpp
  ${CMAKE_SOURCE_DIR}/core/_load-static-noexts.cpp
  ${CMAKE_SOURCE_DIR}/core/prj_trace.cpp
  ${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
  ${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp

  ${CMAKE_SOURCE_DIR}/jimsh/jimshmin.cpp
)

# Smallest possible jimsh
add_executable(prj_compat  
  ${CMAKE_SOURCE_DIR}/vc/prj_compat/prj_compat_main.cpp
  ${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp
)

set_target_properties(jimshpp PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(jimshppmin PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

set(TEST_FILE "${CMAKE_SOURCE_DIR}/tests/testran.fil")

# CMAKE_BUILD_TYPE
# Run the test scripts
add_custom_command(
    COMMENT "Run jimshpp and prj_compat test"
    OUTPUT ${TEST_FILE}
    RESULT_VARIABLE alltest_ret
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
    COMMAND ${CMAKE_COMMAND} -E touch ${TEST_FILE}
    COMMAND ${CMAKE_SOURCE_DIR}/bin/${BUILDIR}prj_compat${CMAKE_EXECUTABLE_SUFFIX_CXX} 
    COMMAND ${CMAKE_SOURCE_DIR}/bin/${BUILDIR}jimshpp${CMAKE_EXECUTABLE_SUFFIX_CXX} runall.tcl
)
if(alltest_ret) 
    message("ERROR: return alltest.tcl")
    set(alltest_ret 0)
endif()

# ALL
add_custom_target(alltest ALL
  DEPENDS ${TEST_FILE} jimshpp prj_compat
)


