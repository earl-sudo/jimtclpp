
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
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-sockets.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-ssl.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext.cpp
)
add_library(jim-aio-extso SHARED 
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-sockets.cpp
${CMAKE_SOURCE_DIR}/binary_ext/jim-aio-ext-ssl.cpp
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
