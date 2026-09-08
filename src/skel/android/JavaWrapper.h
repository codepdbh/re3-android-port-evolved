//
// Ported from the reVC Android port (codepdbh/revc-android-port-evolved).
//

#ifndef RE3_CJAVAWRAPPER_H
#define RE3_CJAVAWRAPPER_H

#if defined ANDROID

#include <jni.h>
#include <string>

#define EXCEPTION_CHECK(env) \
	if ((env)->ExceptionCheck()) \
	{ \
		(env)->ExceptionDescribe(); \
		(env)->ExceptionClear(); \
		return; \
	}

class CJavaWrapper
{
    jmethodID s_ExitGame;
public:
    jobject activity;

    CJavaWrapper(JNIEnv* env, jobject activity);
    ~CJavaWrapper();

    void ExitGame();

    // static methods
    static JNIEnv* GetEnv();
};

extern CJavaWrapper* g_pJavaWrapper;
#endif

#endif //RE3_CJAVAWRAPPER_H
