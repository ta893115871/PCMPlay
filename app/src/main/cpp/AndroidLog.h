//
// Created by Gu,Xiuzhong on 2021/4/13.
//

#ifndef FFMPEGAUDIOPLAYER_ANDROIDLOG_H
#define FFMPEGAUDIOPLAYER_ANDROIDLOG_H


#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"OpenSlEs",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"OpenSlEs",__VA_ARGS__)


#endif //FFMPEGAUDIOPLAYER_ANDROIDLOG_H
