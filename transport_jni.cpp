/*************************************************************************
	> File Name: transport_jni.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年07月07日 星期四 09时52分58秒
 ************************************************************************/

#include <ctype.h>
#include <jni.h>
#include <android_runtime/AndroidRuntime.h>
#include <pthread.h>

#include "fac_log.h"
#include "transport.h"

using namespace android;

#define NELEM(m) (sizeof(m) / sizeof((m)[0]))

#define CLASS_NAME "/com/example/user/SerialActivity"

static jmethodID callbackID = NULL;
static jobject   s_jobject = NULL;
static pthread_t r_thread;

static void hex2Str(char* dst, uint8_t* src, int len)
{
    char dl, dh;

    for (int i = 0; i < len; i++)
    {
        dh = 48 + src[i] / 16;
        dl = 48 + src[i] % 16;
        if (dl > 57) dh = dh + 7;
        if (dl > 57) dl = dl + 7;

        dst[i*2] = dh;
        dst[i*2+1] = dl; 
    }
    dst[len*2] = '\0';
}

static void str2Hex(uint8_t* dst, char* src, int len)
{
    char s1, s2;
    uint8_t d1, d2;
    for (int i = 0; i < len; i++)
    {
        s1 = src[i*2];
        s2 = src[i*2+1];

        d1 = toupper(s1) - 0x30;
        if (d1 > 9) d1 -= 7;

        d2 = toupper(s2) - 0x30;
        if (d2 > 9) d2 -= 7;

        dst[i] = d1 * 16 + d2;
    }
}

static void notifyClient(const char *info)
{
    JNIEnv *env = NULL;

    JavaVM *vm = android::AndroidRuntime::getJavaVM();
    if (vm == NULL)
        return;

    jint result = vm->AttachCurrentThread(&env, NULL);
    if (result != JNI_OK)
        return;

    if (env == NULL) return;

    jstring info_str = env->NewStringUTF(info);
    env->CallVoidMethod(s_jobject, callbackID, info_str);
}

void* receive_thread(void* arg)
{
    uint8_t data[BUF_SIZE];
    int len;
    char str[BUF_SIZE*2 + 1];
    
    for (;;)
    {
        memset(data, 0, sizeof(data));
        len = BUF_SIZE;
        
        getSerialData(data, len);

        if (callbackID)
        {
            hex2Str(str, data, len);
            notifyClient(str);
        }
    }

    return 0;
}

static void init(JNIEnv* env, jobject thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL)
    {
        LOGE("Can't find java class");
        return;
    }

    s_jobject = env->NewGlobalRef(thiz);
    callbackID = env->GetMethodID(clazz, "receiveCallback", "(Ljava/lang/String;)V");

    transport_init();
    pthread_create(&r_thread, NULL, receive_thread, env); 
}

static void uninit(JNIEnv* env, jobject thiz)
{
    void *ret;
    pthread_join(r_thread, &ret);

    callbackID = NULL;

    env->DeleteGlobalRef(s_jobject);
}

static JNINativeMethod gMethods[] =
{
    {"native_serial_init", "()V", (void*)init},
    {"native_serial_uninit", "()V", (void*)uninit},
};

static int register_native_methods(JNIEnv* env)
{
    return AndroidRuntime::registerNativeMethods(env, CLASS_NAME, gMethods, NELEM(gMethods));
}


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    LOGD("Load jni...");

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        LOGE("GetEnv failed");
        goto error;
    }

    assert(env != NULL);

    if (register_native_methods(env) < 0)
    {
        LOGE("registerNativeMethods failed");
        goto error;
    }

    result = JNI_VERSION_1_4;

error:
    return result;
}
