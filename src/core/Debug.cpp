#include "Debug.h"

#if DEBUG_ENABLED
Debug::Level Debug::currentLevel = static_cast<Debug::Level>(DEBUG_LEVEL);
#endif
