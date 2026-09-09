//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//

#if defined ANDROID

#include "AndroidMain.h"
#include "JavaWrapper.h"
#include "logger/log.h"
#include <SDL_hints.h>
#include <SDL_main.h>
#include <SignalHandler.h>
#include <patch/patch.h>

JavaVM* javaVM = NULL;
char* StorageRootBuffer = NULL;
uintptr_t g_libre3 = NULL;

bool AndWrapper::AppInitialized = false;
bool AndWrapper::AppStarted = false;

JAVA_WRAPPER Java_org_libsdl_app_SDLActivity_nativeSetupJNI(JNIEnv* env, jobject initGame)
{
    if(!javaVM) {
        env->GetJavaVM(&javaVM);
    }

    g_pJavaWrapper = new CJavaWrapper(env, initGame);
}

// Kept for parity with reVC's launcher (an alternate JNI path to set the
// storage root); GameActivity.getArguments() (Java side) -> the "--dir"
// command-line parsing in skel/sdl2/sdl2.cpp's main() is the path this
// launcher actually uses and is set up to guarantee StorageRootBuffer gets
// assigned before anything reads it.
JAVA_WRAPPER Java_com_re3_game_core_RE3_setGamePath(JNIEnv *env, jobject obj, jstring value)
{
    const char* root = env->GetStringUTFChars(value, NULL);
    setenv("STORAGE_ROOT", root, 1);

    StorageRootBuffer = getenv("STORAGE_ROOT");
    debug("Storage Root: %s", StorageRootBuffer);

    env->ReleaseStringUTFChars(value, root);
}

bool AndWrapper::InitLibraries() {
	g_libre3 = Patch::FindLib("libre3.so");

	if (!g_libre3) {
		Logger::Log("[ERROR]: Required libraries not found!");
		return false;
	}

	Logger::Log("[INFO]: libre3 base: 0x%X", g_libre3);
	return true;
}

void AndWrapper::TimeInitialize() {
    struct timeval v0;
    gettimeofday(&v0, NULL);
}

void* AndWrapper::GetJNI() {
    return CJavaWrapper::GetEnv();
}

void* AndWrapper::GetJNIFunc() {
    return CJavaWrapper::GetEnv();
}

void* AndWrapper::GetObj() {
    return nullptr;
}

const char* AndWrapper::GetAppId() { return nullptr; }
const char* AndWrapper::GetDeviceID() { return nullptr; }
int AndWrapper::GetDeviceInfo(int index) { return 0; }
bool AndWrapper::IsAppInstalled(const char* app) { return false; }
void AndWrapper::OpenLink(const char* link) {}
bool AndWrapper::DeviceIsTV() { return false; }
int AndWrapper::DeviceLocale() { return 0; }
int AndWrapper::DeviceType() { return 0; }
void AndWrapper::SystemInitialize() {}

JNI_WRAPPER int InitializeGame() {
    debug("Initialize Game");

    if (!AndWrapper::InitLibraries()) return NULL;

    int argc = 0;
    char* argv[1] = { nullptr };

    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    CrashHandler::SetupSignalHandlers();

    int result = SDL_main(argc, argv);
    return result;
}

#endif
