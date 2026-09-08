//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//

#ifndef RE3_SIGNALHANDLER_H
#define RE3_SIGNALHANDLER_H

#if defined ANDROID

#include "common.h"

namespace CrashHandler {
    void SetupSignalHandlers();
}
#endif

#endif //RE3_SIGNALHANDLER_H
