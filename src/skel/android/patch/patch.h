//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//

#ifndef RE3_PATCH_H
#define RE3_PATCH_H

#if defined ANDROID

#include "common.h"
#include <dlfcn.h>

class Patch {
public:
	static uintptr_t FindLib(const char* libName)
	{
#if defined ANDROID
		void* handle = dlopen(libName, RTLD_LAZY);

		if (handle) {
			void* symbol = dlsym(handle, "InitializeGame");
			if (symbol) {
				Dl_info info;
				if (dladdr(symbol, &info) != 0) {
					return reinterpret_cast<uintptr_t>(info.dli_fbase);
				}
			}
			dlclose(handle);
		}
#endif
		return 0;
	}
};
#endif

#endif // RE3_PATCH_H
