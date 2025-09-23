#include "JNITest.h"
#include "correct_main.h"
#include "wrong_main.h"

JNIEXPORT void JNICALL Java_JNITest_runCobolThreadsDefault(JNIEnv *env, jobject thisObject) 
{
    run_cobol_threads_default();
}

JNIEXPORT void JNICALL Java_JNITest_runCobolThreadsWithCustomSignalHandler(JNIEnv *env, jobject thisObject)
{
    run_cobol_threads_with_custom_signal_handler();
}