#pragma once

#include <stddef.h>

#define JIM_NAMESPACE_NAME Jim 
#define BEGIN_JIM_NAMESPACE namespace JIM_NAMESPACE_NAME {
#define END_JIM_NAMESPACE }; /* namespace Jim */

#define BEGIN_NS(X) namespace X {
#define END_NS(X) }; 

// Instead of converting all cast to new C++ cast just mark them with this macro for now.
#define CAST(X) (X)

// Wrap C++17 attribues just in case we don't have them
#define MAYBE_USED [[maybe_unused]]
#define FALLTHROUGH [[fallthrough]]
#define CHKRET [[nodiscard]]

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res

#define __UNIQUE_NAME(base) PP_CAT(base, __COUNTER__)

struct CodePos { // #Debug
    const char* fileName_ = nullptr;
    const char* functName_ = nullptr;
    int lineNum_ = 0;
    CodePos(const char* funcName, int lineNum, const char* functName = nullptr)
        : fileName_(funcName), functName_(functName), lineNum_(lineNum) { }
};
#define CODE_POS_ARGS __FILE__, __LINE__
#define FUNC_POS_ARGS __FILE__, __LINE__, __FUNCTION__
#define CODE_POS CodePos(CODE_POS_ARGS)
#define FUNC_POS CodePos(FUNC_POS_ARGS)

#ifndef JIM_API_INLINE
#  define JIM_API_INLINE  
#endif

#define JIM_IGNORE(X)
