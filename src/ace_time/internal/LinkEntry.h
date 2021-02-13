/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#ifndef ACE_TIME_LINK_ENTRY_H
#define ACE_TIME_LINK_ENTRY_H

#include <stdint.h>

namespace ace_time {

// The data structures in LinkEntry.inc are #included into the basic and
// extended namespaces, instead of subclassing them into the namespaces,
// because C++11 does not allow subclassed structs to be initialized using the
// curly-brace initializers. I believe C++14 removes this restriction but
// Arduino is currently limited to C++11.

namespace basic {
#include "LinkEntry.inc"
}

namespace extended {
#include "LinkEntry.inc"
}

} // ace_time

#endif
