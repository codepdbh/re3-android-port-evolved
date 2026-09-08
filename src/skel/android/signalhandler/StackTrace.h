//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//

#ifndef RE3_STACKTRACE_H
#define RE3_STACKTRACE_H

#if defined ANDROID

#include <dlfcn.h>
#include <execinfo.h>
#include <unwind.h>

extern uintptr_t g_libre3;

#if ANDROID_x32
#define PRINT_CRASH_STATES(context) \
	Logger::CrashLog("register states:"); \
	Logger::CrashLog("r0: 0x%X, r1: 0x%X, r2: 0x%X, r3: 0x%X", (context)->uc_mcontext.arm_r0, (context)->uc_mcontext.arm_r1, (context)->uc_mcontext.arm_r2, (context)->uc_mcontext.arm_r3); \
	Logger::CrashLog("r4: 0x%x, r5: 0x%x, r6: 0x%x, r7: 0x%x", (context)->uc_mcontext.arm_r4, (context)->uc_mcontext.arm_r5, (context)->uc_mcontext.arm_r6, (context)->uc_mcontext.arm_r7); \
	Logger::CrashLog("r8: 0x%x, r9: 0x%x, sl: 0x%x, fp: 0x%x", (context)->uc_mcontext.arm_r8, (context)->uc_mcontext.arm_r9, (context)->uc_mcontext.arm_r10, (context)->uc_mcontext.arm_fp); \
	Logger::CrashLog("ip: 0x%x, sp: 0x%x, lr: 0x%x, pc: 0x%x", (context)->uc_mcontext.arm_ip, (context)->uc_mcontext.arm_sp, (context)->uc_mcontext.arm_lr, (context)->uc_mcontext.arm_pc); \
    Logger::CrashLog("1: libre3.so + 0x%X", context->uc_mcontext.arm_pc - g_libre3); \
    Logger::CrashLog("2: libre3.so + 0x%X", context->uc_mcontext.arm_lr - g_libre3);
#else
#define PRINT_CRASH_STATES(context) \
    Logger::CrashLog("1: libre3.so + 0x%llx", context->uc_mcontext.pc - g_libre3); \
    Logger::CrashLog("2: libre3.so + 0x%llx", context->uc_mcontext.regs[30] - g_libre3);
#endif

class CStackTrace
{
public:
    static void printBacktrace()
    {
        Logger::CrashLog("------------ START BACKTRACE ------------");
        Logger::CrashLog(" ");
        PrintStackTrace();
    }

private:
    static _Unwind_Reason_Code TraceFunction(_Unwind_Context* context, void* arg) {
        uintptr_t pc = _Unwind_GetIP(context);

        Dl_info info;
        if (dladdr(reinterpret_cast<void*>(pc), &info) && info.dli_sname != nullptr) {
            Logger::CrashLog("[adr: %p re3: %p] %s\n",
                     reinterpret_cast<void*>(pc),
                     reinterpret_cast<void*>(pc - g_libre3),
                     info.dli_sname);
        } else {
            Logger::CrashLog("[adr: %p re3: %p] name not found\n",
                     reinterpret_cast<void*>(pc),
                     reinterpret_cast<void*>(pc - g_libre3));
        }

        return _URC_NO_REASON;
    }

    static void PrintStackTrace() {
        _Unwind_Backtrace(TraceFunction, nullptr);
    }

};
#endif

#endif
