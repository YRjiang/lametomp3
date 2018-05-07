//
// Created by jianddongguo on 2017/6/14.
//
#include <android/log.h>
#define LOG_TAG "lamemp3"
// 不带格式Log
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,"%s",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,"%s",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,"%s",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,"%s",__VA_ARGS__)

#define LOG_I(format,...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,format,__VA_ARGS__)
#define LOG_D(format,...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,format,__VA_ARGS__)
#define LOG_W(format,...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,format,__VA_ARGS__)
#define LOG_E(format,...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,format, __VA_ARGS__)
#ifndef LAMETOMP3_LAMEMP3_H
#define LAMETOMP3_LAMEMP3_H


JNIEXPORT void JNICALL
Java_com_teligen_lametomp3_LameMp3_lameInit(JNIEnv *env, jclass type, jint inSampleRate,
jint outChannelNum, jint outSampleRate, jint outBitRate,
        jint quality);

JNIEXPORT jint JNICALL
        Java_com_teligen_lametomp3_LameMp3_lameFlush(JNIEnv *env, jclass type, jbyteArray mp3buf_);

JNIEXPORT void JNICALL
Java_com_teligen_lametomp3_LameMp3_lameClose(JNIEnv *env, jclass type);

JNIEXPORT jint JNICALL
Java_com_teligen_lametomp3_LameMp3_lameEncode(JNIEnv *env, jclass type, jshortArray letftBuf_,
                                              jshortArray rightBuf_, jint sampleRate,
                                              jbyteArray mp3Buf_);

JNIEXPORT void JNICALL
Java_com_teligen_lametomp3_LameMp3_lameConvertmp3(JNIEnv * env, jobject obj, jstring jwav, jstring jmp3);


#endif //LAMETOMP3_LAMEMP3_H
