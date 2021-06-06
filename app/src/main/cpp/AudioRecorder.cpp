//
// Created by guxiuzhong on 5.6.21.
//

#include "AudioRecorder.h"

AudioRecorder::AudioRecorder() {

}

AudioRecorder::~AudioRecorder() {

}

void AudioRecorder::createEngine() {
    SLEngineOption pEngineOptions[] = {(SLuint32) SL_ENGINEOPTION_THREADSAFE,
                                       (SLuint32) SL_BOOLEAN_TRUE};
    // 创建引擎对象,//调用全局方法创建一个引擎对象（OpenSL ES唯一入口）
    SLresult result;
    result = slCreateEngine(
            &engineObject, //对象地址，用于传出对象
            1, //配置参数数量
            pEngineOptions, //配置参数，为枚举数组
            0,  //支持的接口数量
            nullptr, //具体的要支持的接口，是枚举的数组
            nullptr//具体的要支持的接口是开放的还是关闭的，也是一个数组，这三个参数长度是一致的
            );
    assert(SL_RESULT_SUCCESS == result);
    /* Realizing the SL Engine in synchronous mode. */
    //实例化这个对象
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    // get the engine interface, which is needed in order to create other objects
    //从这个对象里面获取引擎接口
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
}

// 录制音频时的回调
void AudioRecorderCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context) {
    //注意这个是另外一条采集线程回调
    AudioRecorder *recorderContext = (AudioRecorder *) context;
    assert(recorderContext != NULL);
    if (recorderContext->buffer != NULL) {
        fwrite(recorderContext->buffer, recorderContext->bufferSize, 1, recorderContext->pfile);
        LOGD("save a frame audio data,pid=%ld", syscall(SYS_gettid));
        SLresult result;
        SLuint32 state;
        result = (*(recorderContext->recorderRecord))->GetRecordState(
                recorderContext->recorderRecord, &state);
        assert(SL_RESULT_SUCCESS == result);
        (void) result;
        LOGD("state=%d", state);
        if (state == SL_RECORDSTATE_RECORDING) {
            //取完数据，需要调用Enqueue触发下一次数据回调
            result = (*bufferQueueItf)->Enqueue(bufferQueueItf, recorderContext->buffer,
                                                recorderContext->bufferSize);
            assert(SL_RESULT_SUCCESS == result);
            (void) result;
        }
    }
}


// 开始采集音频数据，并保存到本地
void AudioRecorder::startRecord() {
    if (engineEngine == nullptr) {
        // 一 创建引擎对象
        createEngine();
    }
    if (recorderObject != nullptr) {
        LOGE("Audio recorder already has been created.");
    }

    pfile = fopen(AUDIO_SRC_PATH, "w");
    if (pfile == NULL) {
        LOGE("Fail to open file.");
        return;
    }
    // OpenSL ES 中的 SLDataSource 和 SLDataSink 结构体，主要用于构建 audio player 和 recorder 对象，
    // 其中 SLDataSource 表示音频数据来源的信息，SLDataSink 表示音频数据输出信息。
    SLresult result;
    /* setup the data source*/
    // 二。设置IO设备(麦克风) 输入输出,我们需要设置采集设备的一些输入输出配置
    SLDataLocator_IODevice ioDevice = {
            SL_DATALOCATOR_IODEVICE,  //类型 这里只能是SL_DATALOCATOR_IODEVICE
            SL_IODEVICE_AUDIOINPUT,//device类型  选择了音频输入类型
            SL_DEFAULTDEVICEID_AUDIOINPUT, //deviceID 对应的是SL_DEFAULTDEVICEID_AUDIOINPUT
            NULL//device实例
    };
    // 数据源
    SLDataSource recSource = {
            &ioDevice,//SLDataLocator_IODevice配置输入
            NULL//输入格式，采集的并不需要
    };
    // 数据源简单缓冲队列定位器,输出buffer队列
    SLDataLocator_AndroidSimpleBufferQueue recBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, //类型 这里只能是SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE
            NUM_BUFFER_QUEUE //buffer的数量
    };
    // PCM 数据源格式 //设置输出数据的格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM, //输出PCM格式的数据
            2,  //  //输出的声道数量2 个声道（立体声）
            SL_SAMPLINGRATE_44_1, //输出的采样频率，这里是44100Hz
            SL_PCMSAMPLEFORMAT_FIXED_16, //输出的采样格式，这里是16bit
            SL_PCMSAMPLEFORMAT_FIXED_16,//一般来说，跟随上一个参数
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//双声道配置，如果单声道可以用 SL_SPEAKER_FRONT_CENTER
            SL_BYTEORDER_LITTLEENDIAN //PCM数据的大小端排列
    };
    // 输出
    SLDataSink dataSink = {
            &recBufferQueue, //SLDataFormat_PCM配置输出
            &pcm //输出数据格式
    };

    // 三 创建录制器 主要是创建录制对象和获取录制相关的接口

    //创建录制的对象，并且指定开放SL_IID_ANDROIDSIMPLEBUFFERQUEUE这个接口
    SLInterfaceID iids[NUM_RECORDER_EXPLICIT_INTERFACES] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                            SL_IID_ANDROIDCONFIGURATION};
    SLboolean required[NUM_RECORDER_EXPLICIT_INTERFACES] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    /* Create the audio recorder */
    // 创建 audio recorder 对象
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, //引擎接口
                                                  &recorderObject, //录制对象地址，用于传出对象
                                                  &recSource,//输入配置
                                                  &dataSink,//输出配置
                                                  NUM_RECORDER_EXPLICIT_INTERFACES,//支持的接口数量
                                                  iids, //具体的要支持的接口
                                                  required //具体的要支持的接口是开放的还是关闭的
    );
    assert(SL_RESULT_SUCCESS == result);

    /* get the android configuration interface*/
    (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDCONFIGURATION, &configItf);
    assert(SL_RESULT_SUCCESS == result);
    /* Realize the recorder in synchronous mode. */ //实例化这个录制对象
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    /* Get the buffer queue interface which was explicitly requested *///获取Buffer接口
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             (void *) &recorderBuffQueueItf);
    assert(SL_RESULT_SUCCESS == result);

    /* get the record interface */ //获取录制接口
    (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);

    // 四。 设置数据回调并且开始录制，设置开始录制状态，并通过回调函数获取录制的音频 PCM 数据：
    buffer = new uint8_t[BUFFER_SIZE]; //数据缓存区，
    bufferSize = BUFFER_SIZE;
    //设置数据回调接口AudioRecorderCallback，最后一个参数是可以传输自定义的上下文引用
    (*recorderBuffQueueItf)->RegisterCallback(recorderBuffQueueItf, AudioRecorderCallback, this);
    assert(SL_RESULT_SUCCESS == result);
    /* Start recording */
    // 开始录制音频，//设置录制器为录制状态 SL_RECORDSTATE_RECORDING
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
    assert(SL_RESULT_SUCCESS == result);

    // 在设置完录制状态后一定需要先Enqueue一次，这样的话才会开始采集回调
    /* Enqueue buffers to map the region of memory allocated to store the recorded data */
    (*recorderBuffQueueItf)->Enqueue(recorderBuffQueueItf, buffer, BUFFER_SIZE);
    assert(SL_RESULT_SUCCESS == result);
    LOGD("Starting recording tid=%ld", syscall(SYS_gettid));//线程id
}

// 停止音频采集
void AudioRecorder::stopRecord() {
    // 停止录制
    if (recorderRecord != nullptr) {
        //设置录制器为停止状态 SL_RECORDSTATE_STOPPED
        SLresult result = result = (*recorderRecord)->SetRecordState(recorderRecord,
                                                                     SL_RECORDSTATE_STOPPED);
        assert(SL_RESULT_SUCCESS == result);
        fclose(pfile);
        pfile = nullptr;
        delete buffer;
        buffer = nullptr;
        LOGD("stopRecord done");
    }
}

// 释放资源,释放OpenSL ES资源
void AudioRecorder::release() {
    //只需要销毁OpenSL ES对象，接口不需要做Destroy处理。
    if (recorderObject != nullptr) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorderRecord = NULL;
        recorderBuffQueueItf = NULL;
        configItf = NULL;
    }
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        // 释放引擎对象的资源
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    LOGD("release done");
}
