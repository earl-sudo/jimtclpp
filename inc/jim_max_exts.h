#pragma once

#define jim_ext_aio 1
#define jim_ext_array 1
#define jim_ext_binary 1
#define jim_ext_clock 1
#define jim_ext_exec 1
#define jim_ext_file 1
#define jim_ext_glob 1
#define jim_ext_history 1
#define jim_ext_interp 1
#define jim_ext_load 1
#define jim_ext_namespace 1
#define jim_ext_nshelper 1
#define jim_ext_oo 1
#define jim_ext_pack 1
#define jim_ext_package 1
#define jim_ext_readdir 1
#define jim_ext_regexp 1
#define jim_ext_stdlib 1
#define jim_ext_tclcompat 1
#define jim_ext_tclprefix 1
#define jim_ext_tree 1
#ifndef _WIN32
#define jim_ext_posix 1
#define jim_ext_signal 1
#define jim_ext_syslog 1
#endif
//#define jim_ext_zlib 1