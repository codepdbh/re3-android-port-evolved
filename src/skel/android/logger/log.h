//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//

#ifndef RE3_LOGGER_H
#define RE3_LOGGER_H

#if defined ANDROID

namespace Logger {
    void Log(const char* fmt, ...);
    void CrashLog(const char* fmt, ...);
}
#endif

#endif
