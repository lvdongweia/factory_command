/*************************************************************************
	> File Name: test_mic.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年08月11日 星期四 14时44分30秒
 ************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <utils/Errors.h>
#include <system/audio.h>
#include <media/mediarecorder.h>
#include <media/mediaplayer.h>
#include <media/AudioSystem.h>
#include <gui/IGraphicBufferProducer.h>
#include <binder/ProcessState.h>

#include "fac_util.h"
#include "fac_errors.h"
#include "test_mic.h"

// --------------------------------------------------------

using namespace android;

// --------------------------------------------------------

#define RECORD_FILE     OUTPUT_PATH "/recording.amr"
#define RING_PLAY_FILE  OUTPUT_PATH "/200-2KHz_-12dB_2cycle.wav"


class _MediaPlayerListener;
static int player_release();
static void set_music_volume(int vl);
static void reset_music_volume();


/* sync with AudioService.java */
static const int STREAM_MUSIC = 3;
static const int DEVICE_OUT_DEFAULT = 0x40000000;

static int STREAM_MUSIC_DEVICE = DEVICE_OUT_DEFAULT; 


static sp<MediaRecorder> mMediaRecorder = NULL;
static int r_fd = 0;
static int p_fd = 0;

static sp<MediaPlayer> mMediaPlayer = NULL;
static sp<_MediaPlayerListener> mMediaPlayerListener = NULL;

static bool isRecording = false;
static bool isPlaying = false;
static int  oldStreamVolume = 0;


class _MediaPlayerListener : public MediaPlayerListener
{
public:
    _MediaPlayerListener() {};
    ~_MediaPlayerListener() {};
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);
};

void _MediaPlayerListener::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    LOGD("player: message received msg=%d, ext1=%d, ext2=%d", msg, ext1, ext2);
    if (msg == MEDIA_PLAYBACK_COMPLETE)
    {
        player_release();
    }
}

int recorder_setup()
{
    status_t ret;
    E_RESET();

    if (mMediaRecorder == NULL)
        mMediaRecorder = new MediaRecorder();

    if((ret = mMediaRecorder->initCheck()) != OK)
    { 
        LOGE("recorder: initCheck error %d", ret);
        E_SET(E_FAILED);
        return -1;
    } 
  
    return 0;
}

int recorder_start()
{
    status_t ret;
    int r_err = E_OK;
    E_RESET();

    if (mMediaRecorder == NULL)
    {
        LOGE("media recorder is not initialized yet");
        r_err = E_DISCONNECTED;
        goto FAILED;
    }

    if (isRecording)
    {
        LOGE("recorder: doing recording");
        r_err = E_EXECUTED;
        goto FAILED;
    }
    
    if ((ret = mMediaRecorder->setAudioSource(AUDIO_SOURCE_MIC)) != OK)
    {
        LOGE("recorder: setAudioSource error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    };
    if ((ret = mMediaRecorder->setOutputFormat(OUTPUT_FORMAT_AMR_NB)) != OK)
    {
        LOGE("recorder: setOutputFormat error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    }
    if ((ret = mMediaRecorder->setAudioEncoder(AUDIO_ENCODER_AMR_NB)) != OK)
    {
        LOGE("recorder: setAudioEncoder error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    }
    
    if ((r_fd = open(RECORD_FILE, O_CREAT|O_WRONLY|O_TRUNC, 0200)) < 0)
    {
        LOGE("recorder: open file %s error", RECORD_FILE);
        r_err = E_FAILED;
        goto FAILED;
    }
    if ((ret = mMediaRecorder->setOutputFile(r_fd, 0, 0)) != OK)
    {
        LOGE("recorder: setOutputFile error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    }
    if ((ret = mMediaRecorder->prepare()) != OK)
    {
        LOGE("recorder: prepare error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    }
    if ((ret = mMediaRecorder->start()) != OK)
    {
        LOGE("recorder: start error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    }

    isRecording = true;
    LOGD("recorder: start recording...");
    return 0;

FAILED:
    E_SET(r_err);
    return -1;
}

int recorder_stop()
{
    status_t ret;
    E_RESET();

    if (mMediaRecorder == NULL)
    {
        LOGE("media recorder is not initialized yet");
        E_SET(E_DISCONNECTED);
        return -1;
    }

    if (!isRecording)
        return 0;

    if ((ret = mMediaRecorder->stop()) != OK)
    {
        LOGE("recorder: stop error: %d", ret);
        E_SET(E_FAILED);
        return -1;
    }

    close(r_fd);
    r_fd = 0;

    isRecording = false;
    LOGD("recorder: recording stopped.");
    return 0;
}

void recorder_release()
{
    status_t ret;
    E_RESET();

    if (mMediaRecorder == NULL)
    {
        LOGE("media recorder is not initialized yet");
        return;
    }

    if ((ret = mMediaRecorder->reset()) != OK)
    {
        LOGE("recorder: reset error: %d", ret);
        E_SET(E_FAILED);
    }

    if ((ret = mMediaRecorder->release()) != OK)
    {
        LOGE("recorder: release error: %d", ret);
        E_SET(E_FAILED);
    }

    mMediaRecorder.clear();
    mMediaRecorder = NULL;

    LOGD("recorder: recorder released.");
    return;
}


static int player_setup(const char* path)
{
    status_t ret;
    int r_err = E_OK;
    E_RESET();

    if (mMediaPlayer == NULL)
    {
        mMediaPlayer = new MediaPlayer();
        mMediaPlayerListener = new _MediaPlayerListener();
    }
    mMediaPlayer->setListener(mMediaPlayerListener);

    ProcessState::self()->startThreadPool();

    IGraphicBufferProducer* const p = NULL;
    sp<IGraphicBufferProducer> st = sp<IGraphicBufferProducer>(p);

    if ((p_fd = open(path, O_RDONLY)) < 0)
    {
        LOGE("player: open file %s error", RECORD_FILE);
        r_err = E_FILE_NOT_FOUND;
        goto FAILED;
    }

    LOGD("player: setDataSource");
    if ((ret = mMediaPlayer->setDataSource(p_fd, 0, 0x7ffffffffffffffL)) != OK)
    {
        LOGE("player: setDataSource error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    }
    
    mMediaPlayer->setVideoSurfaceTexture(st);

    LOGD("player: prepare");
    if ((ret = mMediaPlayer->prepare()) != OK)
    {
        LOGE("player: prepare error: %d", ret);
        r_err = E_FAILED;
        goto FAILED;
    }

    isPlaying = true;

    LOGD("player: setup finish");
    return 0;

FAILED:
    E_SET(r_err);
    return -1;
}

static int player_start()
{
    status_t ret;
    E_RESET();

    if (mMediaPlayer == NULL)
    {
        LOGE("media player is not initialized yet");
        E_SET(E_DISCONNECTED);
        return -1;
    }

    if ((ret = mMediaPlayer->start()) != OK)
    {
        LOGE("player: start error: %d", ret);
        E_SET(E_FAILED);
        return -1;
    }

    LOGD("player: playing record...");
    return 0;
}

static int player_release()
{
    status_t ret;
    //E_RESET();

    if (mMediaPlayer == NULL)
    {
        LOGE("media player is not initialized yet");
        //E_SET(E_DISCONNECTED);
        return -1;
    }
   
    if ((ret = mMediaPlayer->stop()) != OK)
    {
        LOGE("player: stop error: %d", ret);
        //E_SET(E_FAILED);
        //return -1;
    }

    reset_music_volume();
    mMediaPlayer->setListener(0);
    mMediaPlayer->disconnect();

    if ((ret = mMediaPlayer->reset()) != OK)
    {
        LOGE("player: reset error: %d", ret);
        //E_SET(E_FAILED);
        //return -1;
    }

    close(p_fd);

    mMediaPlayer.clear();
    mMediaPlayerListener.clear();

    mMediaPlayer = NULL;
    mMediaPlayerListener = NULL;

    LOGD("player: player released.");
    isPlaying = false;
    return 0;
}

int play_record()
{
    E_RESET();
    if (isPlaying == true)
    {
        E_SET(E_EXECUTED);
        return -1;
    }

    if (player_setup(RECORD_FILE) < 0)
        return -1;

    set_music_volume(MAX_STREAM_MUSIC_VOLUME);

    if (player_start() < 0)
        return -1;

    return 0;
}


static void set_music_volume(int vl)
{
    audio_devices_t device = AudioSystem::getDevicesForStream(static_cast<audio_stream_type_t>(STREAM_MUSIC));
    AudioSystem::getStreamVolumeIndex(static_cast<audio_stream_type_t>(STREAM_MUSIC), &oldStreamVolume, device);
    AudioSystem::setStreamVolumeIndex(static_cast<audio_stream_type_t>(STREAM_MUSIC), vl, device);
    LOGD("Music device:0x%02x volume:%d", (int)device, oldStreamVolume);
}

static void reset_music_volume()
{
    audio_devices_t device = AudioSystem::getDevicesForStream(static_cast<audio_stream_type_t>(STREAM_MUSIC));
    AudioSystem::setStreamVolumeIndex(static_cast<audio_stream_type_t>(STREAM_MUSIC), oldStreamVolume, device);
}


int ring_play_start(PLAY_VOLUME mode, int level)
{
    float l_ch, r_ch;
    E_RESET();
    if (isPlaying == true)
    {
        E_SET(E_EXECUTED);
        return -1;
    }

    if (player_setup(RING_PLAY_FILE) < 0)
        return -1;

    l_ch = (mode & L_ONLY) ? 1.0 : 0.0;
    r_ch = (mode & R_ONLY) ? 1.0 : 0.0;
    LOGD("mode:%d, channel: L-%f, R-%f", mode, l_ch, r_ch);
    mMediaPlayer->setLooping(true);
    mMediaPlayer->setVolume(l_ch, r_ch);

    set_music_volume(level);

    if (player_start() < 0)
        return -1;

    return 0;
}

int ring_play_stop()
{
    E_RESET();
    if (isPlaying == false)
    {
        E_SET(E_DISCONNECTED);
        return -1;
    }

    if (player_release() < 0)
        return -1;

    //reset_music_volume();

    return 0;
}


