#pragma once

#if defined(_DEBUG) // #Debug
#  if defined(_MSC_VER)
#    include <intrin.h>
#    define BREAKPOINT __debugbreak() 
#  endif
#  if defined(PRJ_COMPILER_GCC)
#  define BREAKPOINT __builtin_trap()
#  endif
#else
#  define BREAKPOINT 
#endif
