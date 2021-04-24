package com.bj.gxz.pcmplay;

/**
 * Created by guxiuzhong@baidu.com on 2021/4/13.
 */
public class OpenSlEsPlayer {

    static {
        System.loadLibrary("native-lib");
    }

    public native void init();

    public native void sendPcmData(byte[] data, int size);
    public native void release();

}
