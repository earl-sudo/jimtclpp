#pragma once

#include <jim-base.h>
#include <stdio.h>

// Holder of an ignored return value. Flags such cases 
// for review.
#define IGNORERET MAYBE_USED auto __UNIQUE_NAME(usedreturn) =
// Sometimes IGNORERET has issues so use IGNORERET_BAD. (NOTE: This will leave a warning on compile.)
#define IGNORERET_BAD 

