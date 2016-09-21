/*************************************************************************
	> File Name: test_mic.h
	> Author: 
	> Mail:  
	> Created Time: 2016年08月11日 星期四 14时43分51秒
 ************************************************************************/

#ifndef TEST_MIC_H
#define TEST_MIC_H

/* sync with AudioService.java */
#define MAX_STREAM_MUSIC_VOLUME 15

enum PLAY_VOLUME {
    L_ONLY  = 0x01,
    R_ONLY  = 0x02,
    STEREO  = 0x03
};


int recorder_setup();
int recorder_start();
int recorder_stop();
void recorder_release();

int play_record();

int ring_play_start(PLAY_VOLUME mode, int level);
int ring_play_stop();


#endif /* TEST_MIC_H */
