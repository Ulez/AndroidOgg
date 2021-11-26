#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include "log.h"
#include <string>
#include "fun_learnlife_ogglib_OggLib.h"
#include <string.h>
#include <time.h>
#include <math.h>
#include "vorbis/include/vorbis/vorbisenc.h"
#include "vorbis/include/ogg/ogg.h"

#define READ 1024
signed char onebuffer[READ * 4]; /* out of the data segment, not the stack,-128-127 */

unsigned char outbuffer[READ * 5]; /* out of the data segment, not the stack,-128-127 */

ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
ogg_page og; /* one Ogg bitstream page.  Vorbis packets are inside */
ogg_packet op; /* one raw packet of data for decode */

vorbis_info vi; /* struct that stores all the static vorbis bitstream
                          settings */
vorbis_comment vc; /* struct that stores all the user comments */

vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
vorbis_block vb; /* local working space for packet->PCM decode */

int eos = 0, ret;
int i, founddata;

/**
 * unsigned char 转化为 jbyteArray 返回
 * @param env
 * @param buf
 * @param len
 * @return
 */
jbyteArray as_byte_array(JNIEnv *env, unsigned char *buf, int len) {
    jbyteArray array = env->NewByteArray(len);
    env->SetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte *>(buf));
    return array;
}

/**
 * unsigned char 转化为 jbyteArray 返回
 * @param env
 * @param buf
 * @param len
 * @return
 */
jbyteArray as_byte_array2(JNIEnv *env, unsigned char *buf, int len, unsigned char *buf2, int len2) {
    jbyteArray array = env->NewByteArray(len + len2);
    env->SetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte *>(buf));
    env->SetByteArrayRegion(array, len, len2, reinterpret_cast<jbyte *>(buf2));
    return array;
}

/**
 * jbyteArray 转化为 unsigned char 返回
 * @param env
 * @param array
 * @return
 */
unsigned char *as_unsigned_char_array(JNIEnv *env, jbyteArray array) {
    int len = env->GetArrayLength(array);
    unsigned char *buf = new unsigned char[len + 1];
    env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte *>(buf));
    buf[len] = '\0';
    return buf;
}


_jbyteArray *voiceStart(JNIEnv *env, jbyteArray pArray) {
    /********** Encode setup ************/
    vorbis_info_init(&vi);

    ret = vorbis_encode_init_vbr(&vi, 2, 44100, 0.1);
    if (ret)exit(1);
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "encoder_example.c");

    /* set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    srand(time(NULL));
    ogg_stream_init(&os, rand());

    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header); /* automatically placed in its own
                                         page */
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        ogg_stream_flush(&os, &og);
        return as_byte_array2(env, og.header, og.header_len, og.body, og.body_len);
    }
}

signed char *ConvertJByteaArrayToChars(JNIEnv *env, jbyteArray bytearray) {
    signed char *chars = NULL;
    jbyte *bytes;
    bytes = env->GetByteArrayElements(bytearray, 0);
    int chars_len = env->GetArrayLength(bytearray);
    chars = new signed char[chars_len + 1];
    memset(chars, 0, chars_len + 1);
    memcpy(chars, bytes, chars_len);
    chars[chars_len] = 0;

    env->ReleaseByteArrayElements(bytearray, bytes, 0);

    return chars;
}

_jbyteArray *voiceEncode(JNIEnv *env, jbyteArray pcm) {
    eos = 0;
    int bytes = env->GetArrayLength(pcm);
    jboolean isCopy;
    jbyte *readBytes = env->GetByteArrayElements(pcm, &isCopy);
    memcpy(onebuffer, readBytes, bytes);
    if (isCopy) {
        env->ReleaseByteArrayElements(pcm, readBytes, 0);
    }
    float **buffer = vorbis_analysis_buffer(&vd, READ);
    for (i = 0; i < bytes / 4; i++) {
        buffer[0][i] = ((onebuffer[i * 4 + 1] << 8) |
                        (0x00ff & (int) onebuffer[i * 4])) / 32768.f;
        buffer[1][i] = ((onebuffer[i * 4 + 3] << 8) |
                        (0x00ff & (int) onebuffer[i * 4 + 2])) / 32768.f;
    }
    vorbis_analysis_wrote(&vd, i);
    int allLength = 0;
    while (vorbis_analysis_blockout(&vd, &vb) == 1) {//这里有问题，。，。，，
        vorbis_analysis(&vb, NULL);
        vorbis_bitrate_addblock(&vb);
        while (vorbis_bitrate_flushpacket(&vd, &op)) {
            ogg_stream_packetin(&os, &op);
            while (!eos) {
                int result = ogg_stream_pageout(&os, &og);
                if (result == 0)break;
                memcpy(outbuffer + allLength, og.header, og.header_len);
                allLength += og.header_len;
                memcpy(outbuffer + allLength, og.body, og.body_len);
                allLength += og.body_len;
                if (ogg_page_eos(&og))eos = 1;
            }
        }
    }
    return as_byte_array(env, outbuffer, allLength);
}

_jbyteArray *voiceEnd(JNIEnv *env, jbyteArray pArray) {
    vorbis_analysis_wrote(&vd, 0);
    /* clean up and exit.  vorbis_info_clear() must be called last */
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
    LOGI("byte[] to ogg end---");
    return pArray;
}


extern "C" JNIEXPORT jbyteArray JNICALL Java_fun_learnlife_ogglib_OggLib_encode
        (JNIEnv *env, jobject, jbyteArray array, jint) {
    int len = env->GetArrayLength(array);
    switch (len) {
        case 1://音频开始
            LOGI("in c, start, raw java byte len = %d", len);
            return voiceStart(env, array);
        case 2://音频结束
            LOGI("in c, end, raw java byte len = %d", len);
            return voiceEnd(env, array);
        default://音频编码
            LOGI("in c, process, raw java byte len = %d", len);
            return voiceEncode(env, array);
    }
}

signed char file_read_buffer[READ * 4 + 44]; /* out of the data segment, not the stack */
extern "C" JNIEXPORT jstring JNICALL Java_fun_learnlife_ogglib_OggLib_wavToOgg
        (JNIEnv *env, jobject, jstring inpath, jstring outpath){
    const char *in = env->GetStringUTFChars(inpath, 0);
    const char *out = env->GetStringUTFChars(outpath, 0);
    ogg_stream_state os; /* take physical pages, weld into a logical
                          stream of packets */
    ogg_page og; /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet op; /* one raw packet of data for decode */

    vorbis_info vi; /* struct that stores all the static vorbis bitstream
                          settings */
    vorbis_comment vc; /* struct that stores all the user comments */

    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block vb; /* local working space for packet->PCM decode */

    int eos = 0, ret;
    int i, founddata;
    freopen(in, "r", stdin);
    freopen(out, "w", stdout);

    file_read_buffer[0] = '\0';
    for (i = 0, founddata = 0; i < 30 && !feof(stdin) && !ferror(stdin); i++) {
        fread(file_read_buffer, 1, 2, stdin);

        if (!strncmp((char *) file_read_buffer, "da", 2)) {
            founddata = 1;
            fread(file_read_buffer, 1, 6, stdin);
            break;
        }
    }
    /********** Encode setup ************/
    vorbis_info_init(&vi);
    ret = vorbis_encode_init_vbr(&vi, 2, 44100, 1);
    if (ret)exit(1);
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "encoder_example.c");
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);
    srand(time(NULL));
    ogg_stream_init(&os, rand());
    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header); /* automatically placed in its own
                                         page */
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);
        while (!eos) {
            int result = ogg_stream_flush(&os, &og);
            if (result == 0)break;
            fwrite(og.header, 1, og.header_len, stdout);
            fwrite(og.body, 1, og.body_len, stdout);
        }

    }

    while (!eos) {
        long i;
        long bytes = fread(file_read_buffer, 1, READ * 4, stdin); /* stereo hardwired here */
        if (bytes == 0) {
            vorbis_analysis_wrote(&vd, 0);
        } else {
            float **buffer = vorbis_analysis_buffer(&vd, READ);
            for (i = 0; i < bytes / 4; i++) {
                buffer[0][i] = ((file_read_buffer[i * 4 + 1] << 8) |
                                (0x00ff & (int) file_read_buffer[i * 4])) / 32768.f;
                buffer[1][i] = ((file_read_buffer[i * 4 + 3] << 8) |
                                (0x00ff & (int) file_read_buffer[i * 4 + 2])) / 32768.f;
            }
            vorbis_analysis_wrote(&vd, i);
        }
        while (vorbis_analysis_blockout(&vd, &vb) == 1) {
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);
            while (vorbis_bitrate_flushpacket(&vd, &op)) {
                ogg_stream_packetin(&os, &op);
                while (!eos) {
                    int result = ogg_stream_pageout(&os, &og);
                    if (result == 0)break;
                    fwrite(og.header, 1, og.header_len, stdout);
                    fwrite(og.body, 1, og.body_len, stdout);
                    if (ogg_page_eos(&og)) {
                        eos = 1;
                    }
                }
            }
        }
    }
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
    env->ReleaseStringUTFChars(inpath, in);
    env->ReleaseStringUTFChars(outpath, out);
    LOGI("wav to ogg file end---");
    return outpath;
}




