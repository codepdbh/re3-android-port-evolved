//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//
// NOTE: the reVC version also logged a handful of "last processed X" globals
// (last streamed model index, last collision entity, etc.) that reVC added
// itself in various .cpp files for earlier crash debugging. This re3
// checkout doesn't have any of those globals, so they're left out here --
// the signal handler, register dump and backtrace below don't depend on them.
//

#if defined ANDROID

#include "SignalHandler.h"
#include "../logger/log.h"
#include "StackTrace.h"

#include <csignal>
#include <string.h>
#include <time.h>
#include <ucontext.h>

namespace CrashHandler {
    static struct sigaction oldHandlers[4]; // 0: SEGV, 1: ABRT, 2: FPE, 3: BUS

    void PrintBuildCrashInfo()
    {
        time_t currentTime = time(nullptr);
        tm* timeInfo = localtime(&currentTime);

        Logger::CrashLog("Crash time: %d:%d:%d %d:%d:%d", timeInfo->tm_mday, timeInfo->tm_mon, timeInfo->tm_year, timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
        Logger::CrashLog("Build times: %s %s. ABI: %s", __TIME__, __DATE__, (ANDROID_x32 ? "armeabi-v7a" : "arm64-v8a"));
    }

    void SignalHandler(int signum, siginfo_t* info, void* contextPtr) {
        auto* context = static_cast<ucontext_t*>(contextPtr);

        struct sigaction* oldHandler = nullptr;
        const char* signalName = nullptr;

        switch (signum) {
            case SIGSEGV:
                oldHandler = &oldHandlers[0];
                signalName = "SIGSEGV";
                Logger::CrashLog(" ");
                break;
            case SIGABRT:
                oldHandler = &oldHandlers[1];
                signalName = "SIGABRT";
                Logger::CrashLog(" ");
                break;
            case SIGFPE:
                oldHandler = &oldHandlers[2];
                signalName = "SIGFPE";
                break;
            case SIGBUS:
                oldHandler = &oldHandlers[3];
                signalName = "SIGBUS";
                break;
            default:
                Logger::CrashLog("Unhandled signal: %d", signum);
                return;
        }

        if (oldHandler && oldHandler->sa_sigaction) {
            oldHandler->sa_sigaction(signum, info, contextPtr);
        }

        PrintBuildCrashInfo();
        Logger::CrashLog("%s | Fault address: 0x%X", signalName, info->si_addr);
        PRINT_CRASH_STATES(context);
        CStackTrace::printBacktrace();
    }

    void SetupSignalHandlers() {
        struct {
            int signal;
            int index;
        } signals[] = {
                { SIGSEGV, 0 },
                { SIGABRT, 1 },
                { SIGFPE,  2 },
                { SIGBUS,  3 },
        };

        for (const auto& s : signals) {
            struct sigaction act {};
            act.sa_sigaction = SignalHandler;
            sigemptyset(&act.sa_mask);
            act.sa_flags = SA_SIGINFO;
            sigaction(s.signal, &act, &oldHandlers[s.index]);
        }
    }
}
#endif
