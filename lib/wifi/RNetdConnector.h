#ifndef __RNETD_CONNECTOR_H_
#define __RNETD_CONNECTOR_H_

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

class RNetdConnectorListener
{
public:
    virtual ~RNetdConnectorListener();

    void onDaemonConnected();
    void onEvent(int code);
};

class RNetdConnector
{
public:
    class RCommand
    {
    public:
        RCommand(const char* cmd, int SN);
        ~RCommand();

        int appendArg(const char* arg);
        int getArgNum();
        char** getArgArray();

    public:
        static const int ARG_LEN = 32;
        static const int CMD_LEN = 16;
        static const int ARG_NUM = 32;

    private:
        RCommand();
        void initData();

        char mCmd[CMD_LEN];
        char mArgArray[ARG_NUM][ARG_LEN];
        char *mArgRet[ARG_NUM];
        int mArgNum;
    };

    RNetdConnector();
    ~RNetdConnector();

    void setListener(RNetdConnectorListener* listener);

    int startMonitoring();
    void stopMonitoring();

    int doCommand(int argc, char **argv);

private:

    static void* doMonitoring(void* arg);

    int mSocket;
    bool mIsMonitoring;

    pthread_t mMonitorHandle;
    pthread_mutex_t mMutex;

    RNetdConnectorListener* mListener;
};

#endif
