//
// Created by guxiuzhong on 5.6.21.
//

#ifndef PCMPLAY_AUDIORECORDER_H
#define PCMPLAY_AUDIORECORDER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <assert.h>
#include "AndroidLog.h"
#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#define  AUDIO_SRC_PATH "/sdcard/audio.pcm"
#define NUM_BUFFER_QUEUE 1
#define  SAMPLE_RATE 44100
#define NUM_RECORDER_EXPLICIT_INTERFACES 2
#define PERIOD_TIME 20  // 20ms
#define FRAME_SIZE SAMPLE_RATE * PERIOD_TIME / 1000
#define CHANNELS 2
#define BUFFER_SIZE   (FRAME_SIZE * CHANNELS)

class AudioRecorder {

public:
    AudioRecorder();

    ~AudioRecorder();

public:
    void startRecord();

    void stopRecord();

    void release();

private:
    SLEngineItf engineEngine = NULL;
    SLObjectItf engineObject = NULL;
    SLObjectItf recorderObject = NULL; //录制对象，这个对象我们从里面获取了2个接口
    SLAndroidConfigurationItf configItf = NULL;
    SLAndroidSimpleBufferQueueItf recorderBuffQueueItf = NULL; //Buffer接口
    void createEngine();

public:
    uint8_t *buffer;
    FILE *pfile;
    size_t bufferSize;
    SLRecordItf recorderRecord = NULL; //录制接口
};


#endif //PCMPLAY_AUDIORECORDER_H
