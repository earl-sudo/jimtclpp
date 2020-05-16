#pragma once

#define JIM_NAMESPACE_NAME Jim 
#define BEGIN_JIM_NAMESPACE namespace JIM_NAMESPACE_NAME {
#define END_JIM_NAMESPACE }; /* namespace Jim */

#define BEGIN_NS(X) namespace X {
#define END_NS(X) }; 

#define CAST(X) (X)

// Wrap C++17 attribues just in case we don't have them
#define MAYBE_USED [[maybe_unused]]
#define FALLTHROUGH [[fallthrough]]
#define CHKRET [[nodiscard]]

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res

#define __UNIQUE_NAME(base) PP_CAT(base, __COUNTER__)

// Holder of an ignored return value. Flags such cases 
// for review.
#define IGNORERET MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
// Sometimes IGNORERET has issues so use IGNORERET_BAD. (NOTE: This will leave a warning on compile.)
#define IGNORERET_BAD  

