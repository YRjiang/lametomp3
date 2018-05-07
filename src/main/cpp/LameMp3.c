//
// Created by jianddongguo on 2017/6/14.
//
#include <jni.h>
#include "LameMp3.h"
#include "lame/lame.h"


/*yr add*/
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

static lame_global_flags *gfp = NULL;

/*yr add*/
// java中的jstring, 转化为c的一个字符数组
char* Jstring2CStr(JNIEnv* env, jstring jstr)
{
    char*   rtn   =   NULL;
    jclass   clsstring   =   (*env)->FindClass(env,"java/lang/String");
    jstring   strencode   =   (*env)->NewStringUTF(env,"GB2312");
    jmethodID   mid   =   (*env)->GetMethodID(env,clsstring,   "getBytes",   "(Ljava/lang/String;)[B");
    jbyteArray   barr=   (jbyteArray)(*env)->CallObjectMethod(env,jstr,mid,strencode); // String .getByte("GB2312");
    jsize   alen   =   (*env)->GetArrayLength(env,barr);
    jbyte*   ba   =   (*env)->GetByteArrayElements(env,barr,JNI_FALSE);
    if(alen   >   0)
    {
        rtn   =   (char*)malloc(alen+1);         //new   char[alen+1]; "\0"
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }
    (*env)->ReleaseByteArrayElements(env,barr,ba,0);  //释放内存

    return rtn;
}


JNIEXPORT void JNICALL
Java_com_teligen_lametomp3_LameMp3_lameInit(JNIEnv *env, jclass type, jint inSampleRate,
jint outChannelNum, jint outSampleRate, jint outBitRate,
        jint quality) {
    if(gfp != NULL){
        lame_close(gfp);
        gfp = NULL;
    }
    //  初始化
    gfp = lame_init();
    LOGI("初始化lame库完成");
    //配置参数
    lame_set_in_samplerate(gfp,inSampleRate);
    lame_set_num_channels(gfp,outChannelNum);
    lame_set_out_samplerate(gfp,outSampleRate);
    lame_set_brate(gfp,outBitRate);
    lame_set_quality(gfp,quality);
    lame_init_params(gfp);
    LOGI("配置lame参数完成");
}

JNIEXPORT jint JNICALL
        Java_com_teligen_lametomp3_LameMp3_lameFlush(JNIEnv *env, jclass type, jbyteArray mp3buf_) {
    jbyte *mp3buf = (*env)->GetByteArrayElements(env, mp3buf_, NULL);
    jsize len = (*env)->GetArrayLength(env,mp3buf_);
    int resut = lame_encode_flush(gfp,mp3buf,len);
    (*env)->ReleaseByteArrayElements(env, mp3buf_, mp3buf, 0);
    LOG_I("写入mp3数据到文件，返回结果=%d",resut);
    return  resut;
}

JNIEXPORT void JNICALL
Java_com_teligen_lametomp3_LameMp3_lameClose(JNIEnv *env, jclass type) {
    lame_close(gfp);
    gfp = NULL;
    LOGI("释放lame资源");
}

JNIEXPORT jint JNICALL
Java_com_teligen_lametomp3_LameMp3_lameEncode(JNIEnv *env, jclass type, jshortArray letftBuf_,
                                              jshortArray rightBuf_, jint sampleRate,
                                              jbyteArray mp3Buf_) {
    if(letftBuf_ == NULL || mp3Buf_ == NULL){
        LOGI("letftBuf和rightBuf 或mp3Buf_不能为空");
        return -1;
    }
    jshort *letftBuf = NULL;
    jshort *rightBuf = NULL;
    if(letftBuf_ != NULL){
        letftBuf = (*env)->GetShortArrayElements(env, letftBuf_, NULL);
    }
    if(rightBuf_ != NULL){
        rightBuf = (*env)->GetShortArrayElements(env, rightBuf_, NULL);
    }
    jbyte *mp3Buf = (*env)->GetByteArrayElements(env, mp3Buf_, NULL);
    jsize readSizes = (*env)->GetArrayLength(env,mp3Buf_);
    // 编码
    int result = lame_encode_buffer(gfp,letftBuf,rightBuf,sampleRate,mp3Buf,readSizes);

    // 释放资源
    if(letftBuf_ != NULL){
        (*env)->ReleaseShortArrayElements(env, letftBuf_, letftBuf, 0);
    }
    if(rightBuf_ != NULL){
        (*env)->ReleaseShortArrayElements(env, rightBuf_, rightBuf, 0);
    }
    (*env)->ReleaseByteArrayElements(env, mp3Buf_, mp3Buf, 0);
    LOG_I("编码pcm为mp3，数据长度=%d",result);
    return  result;
}

/**
 * wav转换mp3
 */
int flag = 0;

JNIEXPORT void JNICALL Java_com_teligen_lametomp3_LameMp3_lameConvertmp3
        (JNIEnv * env, jobject obj, jstring jwav, jstring jmp3) {
    char* cwav =Jstring2CStr(env,jwav) ;
    char* cmp3=Jstring2CStr(env,jmp3);
    LOGI("amr = %s", cwav);
    LOGI("mp3 = %s", cmp3);

    //1.打开 wav,MP3文件
    FILE* fwav = fopen(cwav,"rb");
    FILE* fmp3 = fopen(cmp3,"wb");

    short int wav_buffer[8192*2];
    unsigned char mp3_buffer[8192];

    //yr add
    /*

    //录音设置
        _recorderSettings = [[NSDictionary alloc] initWithObjectsAndKeys:
                             [NSNumber numberWithInt:kAudioFormatLinearPCM],AVFormatIDKey,
                             [NSNumber numberWithInt:11025.0],AVSampleRateKey,
                             [NSNumber numberWithInt:2],AVNumberOfChannelsKey,
                             [NSNumber numberWithInt:AVAudioQualityMin],AVEncoderAudioQualityKey,
                             nil];

    */


    //1.初始化lame的编码器
    lame_t lame =  lame_init();
    //2. 设置lame mp3编码的采样率
    lame_set_in_samplerate(lame , 8000);
    lame_set_num_channels(lame,1);
    lame_set_out_samplerate(lame, 44100);
    lame_set_brate(lame, 32);
    lame_set_quality(lame, 5);
    // 3. 设置MP3的编码方式
    lame_set_VBR(lame, vbr_default);
    lame_set_findReplayGain(lame, 0);
    lame_init_params(lame);


/*    //1.初始化lame的编码器
    lame_t lame =  lame_init();
    //2. 设置lame mp3编码的采样率
    lame_set_VBR(lame, vbr_default);
    lame_set_in_samplerate(lame , 20000);
    lame_set_num_channels(lame,2);
    lame_set_out_samplerate(lame, 20000);
    lame_set_brate(lame, 8);
    lame_set_mode(lame,3);
    lame_set_quality(lame, 5);
    // 3. 设置MP3的编码方式
    //lame_set_VBR(lame, vbr_default);
    lame_init_params(lame);*/


/*    //1.初始化lame的编码器
    lame_t lame =  lame_init();
    //2. 设置lame mp3编码的采样率
    lame_set_in_samplerate(lame , 44100);
    lame_set_num_channels(lame,2);
    // 3. 设置MP3的编码方式
    lame_set_VBR(lame, vbr_default);
    lame_init_params(lame);*/


    LOGI("lame init finish");
    int read ; int write; //代表读了多少个次 和写了多少次
    int total=0; // 当前读的wav文件的byte数目
    do{
        if( flag == 404){
            return;
        }
        read = fread(wav_buffer,sizeof(short int)*2, 8192,fwav);
        total +=  read* sizeof(short int)*2;
        LOGI("converting ....%d", total);
        if(read!=0){
            write = lame_encode_buffer_interleaved(lame,wav_buffer,read,mp3_buffer,8192);
            //把转化后的mp3数据写到文件里
            fwrite(mp3_buffer,sizeof(unsigned char),write,fmp3);
        }
        if(read==0){
            lame_encode_flush(lame,mp3_buffer,8192);
        }
    }while(read!=0);
    LOGI("convert  finish");

    lame_close(lame);
    fclose(fwav);
    fclose(fmp3);
}


/*yr add test - 2*/

#define LOG_TAG "LAME ENCODER"
#define LOGD(format, args...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, ##args);
#define BUFFER_SIZE 8192
#define be_short(s) ((short) ((unsigned short) (s) << 8) | ((unsigned short) (s) >> 8))

lame_t lame;

int read_samples(FILE *input_file, short *input) {
    int nb_read;
    nb_read = fread(input, 1, sizeof(short), input_file) / sizeof(short);

    int i = 0;
    while (i < nb_read) {
        input[i] = be_short(input[i]);
        i++;
    }

    return nb_read;
}

void Java_com_samsung_sample_lame4android_LameActivity_initEncoder(JNIEnv *env,
                                                                   jobject jobj, jint in_num_channels, jint in_samplerate, jint in_brate,
                                                                   jint in_mode, jint in_quality) {
    lame = lame_init();

    LOGD("Init parameters:");
    lame_set_num_channels(lame, in_num_channels);
    LOGD("Number of channels: %d", in_num_channels);
    lame_set_in_samplerate(lame, in_samplerate);
    LOGD("Sample rate: %d", in_samplerate);
    lame_set_brate(lame, in_brate);
    LOGD("Bitrate: %d", in_brate);
    lame_set_mode(lame, in_mode);
    LOGD("Mode: %d", in_mode);
    lame_set_quality(lame, in_quality);
    LOGD("Quality: %d", in_quality);

    int res = lame_init_params(lame);
    LOGD("Init returned: %d", res);
}

void Java_com_samsung_sample_lame4android_LameActivity_destroyEncoder(
        JNIEnv *env, jobject jobj) {
    int res = lame_close(lame);
    LOGD("Deinit returned: %d", res);
}

void Java_com_samsung_sample_lame4android_LameActivity_encodeFile(JNIEnv *env,
                                                                  jobject jobj, jstring in_source_path, jstring in_target_path) {
    const char *source_path, *target_path;
    source_path = (*env)->GetStringUTFChars(env, in_source_path, NULL);
    target_path = (*env)->GetStringUTFChars(env, in_target_path, NULL);

    FILE *input_file, *output_file;
    input_file = fopen(source_path, "rb");
    output_file = fopen(target_path, "wb");

    short input[BUFFER_SIZE];
    char output[BUFFER_SIZE];
    int nb_read = 0;
    int nb_write = 0;
    int nb_total = 0;

    LOGD("Encoding started");
    while (nb_read = read_samples(input_file, input)) {
        nb_write = lame_encode_buffer(lame, input, input, nb_read, output,
                                      BUFFER_SIZE);
        fwrite(output, nb_write, 1, output_file);
        nb_total += nb_write;
    }
    LOGD("Encoded %d bytes", nb_total);

    nb_write = lame_encode_flush(lame, output, BUFFER_SIZE);
    fwrite(output, nb_write, 1, output_file);
    LOGD("Flushed %d bytes", nb_write);

    fclose(input_file);
    fclose(output_file);
}