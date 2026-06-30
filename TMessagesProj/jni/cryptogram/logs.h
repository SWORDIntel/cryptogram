#pragma once
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "CRYPTOGRAM", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "CRYPTOGRAM", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "CRYPTOGRAM", __VA_ARGS__)
