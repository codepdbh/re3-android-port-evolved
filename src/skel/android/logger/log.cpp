//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//

#if defined ANDROID

#include "common.h"
#include "log.h"
#include "AndroidMain.h"
#include <android/log.h>

extern char* StorageRootBuffer;

void Logger::Log(const char *fmt, ...)
{
    static char buffer[512]{};

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);
#if defined ANDROID
    __android_log_write(ANDROID_LOG_INFO, "LOG", buffer);
#endif
#ifdef USE_FILE_LOG
    static FILE* flLog = nullptr;

    if(flLog == nullptr && StorageRootBuffer != nullptr) {
        char path[600];
        snprintf(path, sizeof(path), "%s/log.txt", StorageRootBuffer);
        flLog = fopen(path, "ab");
    }

    if(flLog == nullptr) return;
    fprintf(flLog, "%s\n", buffer);
    fflush(flLog);
#endif
}

void Logger::CrashLog(const char* fmt, ...)
{
    static char buffer[512]{};
    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);
#if defined ANDROID
    __android_log_write(ANDROID_LOG_FATAL, "CRASH LOG", buffer);
#endif
#ifdef USE_FILE_LOG
    static FILE* flLog = nullptr;

    if (flLog == nullptr && StorageRootBuffer != nullptr) {
        char path[600];
        snprintf(path, sizeof(path), "%s/crash_log.txt", StorageRootBuffer);
        flLog = fopen(path, "ab");
    }

    if (flLog == nullptr) return;
    fprintf(flLog, "%s\n", buffer);
    fflush(flLog);
#endif
}
#endif
