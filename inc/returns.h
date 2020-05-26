#pragma once

#include <jim-base.h>
#include <stdio.h>

// Holder of an ignored return value. Flags such cases 
// for review.
#define IGNORERET MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
// For Retval type
#define IGNOREJIMRET MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
// Posix 0 is success
// Posix -1 is failure
#define IGNOREPOSIXRET MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
// Null is failure
#define IGNOREPTRRET MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
// No real error 
#define IGNOREEXTRADATA  MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
#define IGNORE_IMPOS_ERROR MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
#define IGNORE_NOREAL_ERROR MAYBE_USED auto __UNIQUE_NAME(usedreturn) =

// Sometimes IGNORERET has issues so use IGNORERET_BAD. (NOTE: This will leave a warning on compile.)
#define IGNORERET_BAD 

