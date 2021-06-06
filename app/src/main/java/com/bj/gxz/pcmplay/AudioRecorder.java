package com.bj.gxz.pcmplay;

/**
 * Created by guxiuzhong on 2021/06/05 2:03 下午
 */
public class AudioRecorder {
    static {
        System.loadLibrary("native-lib");
    }

    public native void startRecord();

    public native void stopRecord();

    public native void release();
}
