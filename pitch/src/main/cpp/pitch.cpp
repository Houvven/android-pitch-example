#include <jni.h>
#include <memory>
#include "pitch_detector.h"

static std::unique_ptr<PitchDetector> detector;
static JavaVM *javaVM = nullptr;
static jobject globalCallback = nullptr;

// 保存 JavaVM 的引用
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    return JNI_VERSION_1_6;
}

void onPitchDetected(float pitch) {
    JNIEnv *env;
    if (javaVM->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        // 获取回调接口类
        jclass callbackClass = env->GetObjectClass(globalCallback);
        // 获取回调方法ID
        jmethodID onPitchMethod = env->GetMethodID(callbackClass, "onPitchDetected", "(F)V");
        // 调用回调方法
        env->CallVoidMethod(globalCallback, onPitchMethod, pitch);

        javaVM->DetachCurrentThread();
    }
}

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_houvven_pitch_PitchDetectionNative_startPitchDetection(
        JNIEnv *env,
        jobject /* this */,
        jobject callback) {

    // 保存全局回调引用
    if (globalCallback != nullptr) {
        env->DeleteGlobalRef(globalCallback);
    }
    globalCallback = env->NewGlobalRef(callback);

    if (!detector) {
        detector = std::make_unique<PitchDetector>([](float pitch) {
            onPitchDetected(pitch);
        });
    }
    return static_cast<jboolean>(detector->start());
}

JNIEXPORT jboolean JNICALL
Java_com_houvven_pitch_PitchDetectionNative_stopPitchDetection(
        JNIEnv *env,
        jobject /* this */) {
    if (detector) {
        detector->stop();

        // 清理全局引用
        if (globalCallback != nullptr) {
            env->DeleteGlobalRef(globalCallback);
            globalCallback = nullptr;
        }

        return JNI_TRUE;
    }
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_houvven_pitch_PitchDetectionNative_isRunning(
        JNIEnv *env,
        jobject /* this */) {
    if (detector) {
        return static_cast<jboolean>(detector->isRunning());
    }
    return JNI_FALSE;
}

}
