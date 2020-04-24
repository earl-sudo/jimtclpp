#pragma once


#include <jim-base.h>

BEGIN_JIM_NAMESPACE

/**
 * Returns the canonical name for the given signal,
 * e.g. "SIGTERM", "SIGINT"
 */
const char *Jim_SignalId(int sig);


END_JIM_NAMESPACE