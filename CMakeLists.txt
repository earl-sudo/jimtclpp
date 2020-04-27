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

# Add options to compiler
# set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_UTF8=1")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile time info (just example code)
# configure_file("${CMAKE_SOURCE_DIR}/inc/jim_info.h.in" "${CMAKE_SOURCE_DIR}/inc/jim_info.h")

# Build static library jimpp
add_library(jimpp STATIC
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
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
${CMAKE_SOURCE_DIR}/core/utf8.cpp
${CMAKE_SOURCE_DIR}/core/_load-static-exts.cpp
${CMAKE_SOURCE_DIR}/portabilty/jim-win32compat.cpp
${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp
${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tree-sext.cpp
)

# Build code extensions===========================
add_library(jim-regexp-ext STATIC 
${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
)
add_library(jim-regexp-extso SHARED 
${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
)
add_library(jim-signal-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-signal-ext.cpp
)
add_library(jim-signal-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-signal-ext.cpp
)
add_library(jim-aio-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
)
add_library(jim-aio-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
)
add_library(jim-posix-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-posix-ext.cpp
)
add_library(jim-posix-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-posix-ext.cpp
)
add_library(jim-array-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
)
add_library(jim-array-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
)
add_library(jim-clock-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
)
add_library(jim-clock-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
)
add_library(jim-eventloop-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
)
add_library(jim-eventloop-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
)
add_library(jim-exec-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
)
add_library(jim-exec-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
)
add_library(jim-file-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
)
add_library(jim-file-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
)
add_library(jim-history-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-history-ext.cpp
)
add_library(jim-history-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-history-ext.cpp
)
add_library(jim-interp-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-interp-ext.cpp
)
add_library(jim-interp-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-interp-ext.cpp
)
add_library(jim-namespace-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-namespace-ext.cpp
)
add_library(jim-namespace-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-namespace-ext.cpp
)
add_library(jim-pack-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-pack-ext.cpp
)
add_library(jim-pack-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-pack-ext.cpp
)
add_library(jim-package-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-package-ext.cpp
)
add_library(jim-package-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-package-ext.cpp
)
add_library(jim-readdir-ext STATIC 
${CMAKE_SOURCE_DIR}/binary_ext/jim-readdir-ext.cpp
)
add_library(jim-readdir-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-readdir-ext.cpp
)

# build src extensions ===========================
add_library(jim-binary-ext STATIC 
${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
)
add_library(jim-binary-extso SHARED 
${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
)
add_library(jim-glob-ext STATIC 
${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp
)
add_library(jim-glob-extso SHARED 
${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp
)
add_library(jim-initjimsh-ext STATIC 
${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
)
add_library(jim-initjimsh-extso SHARED 
${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
)
add_library(jim-nshelper-ext STATIC 
${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
)
add_library(jim-nshelper-extso SHARED 
${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
)
add_library(jim-oo-ext STATIC 
${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
)
add_library(jim-oo-extso SHARED 
${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
)
add_library(jim-stdlib-ext STATIC 
${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
)
add_library(jim-stdlib-extso SHARED 
${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
)
add_library(jim-tclcompat-ext STATIC 
${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
)
add_library(jim-tclcompat-extso SHARED 
${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
)

# Left over files - for later
# ${CMAKE_SOURCE_DIR}/portabilty/jim-tty.cpp
# ${CMAKE_SOURCE_DIR}/portabilty/jim-zlib.cpp


# Build shared library jimppso
add_library(jimppso SHARED
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
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
${CMAKE_SOURCE_DIR}/core/utf8.cpp
${CMAKE_SOURCE_DIR}/core/_load-static-exts.cpp
${CMAKE_SOURCE_DIR}/portabilty/jim-win32compat.cpp
${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp
${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tree-sext.cpp
)

# TODO How to build debug jimshd?

# TODO Use library in build of jimshpp
# target_link_libraries(Tutorial PUBLIC ${EXTRA_LIBS})

# Build executable jimsh
add_executable(jimshpp 
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-array-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-clock-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-eventloop-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-exec-ext.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-file-ext.cpp
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
${CMAKE_SOURCE_DIR}/core/utf8.cpp
${CMAKE_SOURCE_DIR}/core/_load-static-exts.cpp
${CMAKE_SOURCE_DIR}/portabilty/jim-win32compat.cpp
${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp
${CMAKE_SOURCE_DIR}/regexp/jim-regexp-ext.cpp
${CMAKE_SOURCE_DIR}/regexp/jimregexp.cpp
${CMAKE_SOURCE_DIR}/script_ext/_binary-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_glob-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_initjimsh-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_nshelper-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_oo-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_stdlib-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tclcompat-sext.cpp
${CMAKE_SOURCE_DIR}/script_ext/_tree-sext.cpp

${CMAKE_SOURCE_DIR}/jimsh/jimsh.cpp
)

add_executable(jimshppmin 
  ${CMAKE_SOURCE_DIR}/core/jim-format.cpp
  ${CMAKE_SOURCE_DIR}/core/jim-interactive.cpp
  ${CMAKE_SOURCE_DIR}/core/jim-subcmd.cpp
  ${CMAKE_SOURCE_DIR}/core/jim.cpp
  ${CMAKE_SOURCE_DIR}/core/utf8.cpp
  ${CMAKE_SOURCE_DIR}/core/_load-static-noexts.cpp
  ${CMAKE_SOURCE_DIR}/portabilty/jim-win32compat.cpp
  ${CMAKE_SOURCE_DIR}/portabilty/jimiocompat.cpp
  ${CMAKE_SOURCE_DIR}/portabilty/prj_compat.cpp

  
  ${CMAKE_SOURCE_DIR}/jimsh/jimshmin.cpp
)
set_target_properties(jimshpp PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

enable_testing()

# does the application run
# add_test(NAME Runs COMMAND jimshpp)

# define a function to simplify adding tests
function(do_test target arg result)
  add_test(NAME Comp${arg} COMMAND ${target} ${arg})
  set_tests_properties(Comp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endfunction(do_test)

# do_test(Tutorial 4 "4 is 2")

