#define LOG_TAG "RNetdConnector"

#include "RNetdConnector.h"
#include "factory_log.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <strings.h>

#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>

RNetdConnector::RCommand::RCommand()
{
    initData();
}

RNetdConnector::RCommand::RCommand(const char* cmd, int SN)
{
    initData();

    char SN_str[16] = {0};
    sprintf(SN_str, "%d", SN);
    appendArg(SN_str);
    appendArg(cmd);

    strncpy(mCmd, cmd, CMD_LEN - 1);
}

RNetdConnector::RCommand::~RCommand()
{
}

void RNetdConnector::RCommand::initData()
{
    memset(mCmd, 0, CMD_LEN);
    for (int i = 0; i < ARG_NUM; ++i)
        memset(mArgArray[i], 0, ARG_LEN);

    mArgNum = 0;
}

int RNetdConnector::RCommand::appendArg(const char* arg)
{
    if (mArgNum >= ARG_NUM)
    {
        LOGE("too many argument");
        return -1;
    }

    strncpy(mArgArray[mArgNum], arg, ARG_LEN - 1);
    mArgRet[mArgNum] = mArgArray[mArgNum];
    mArgNum++;

    return 0;
}

int RNetdConnector::RCommand::getArgNum()
{
    return mArgNum;
}

char** RNetdConnector::RCommand::getArgArray()
{
    return mArgRet;
}

RNetdConnector::RNetdConnector()
{
    mSocket = -1;
    mIsMonitoring = false;

    mListener = NULL;

    memset(&mMonitorHandle, 0, sizeof(pthread_t));
}

RNetdConnector::~RNetdConnector()
{
    stopMonitoring();
}

void RNetdConnector::setListener(RNetdConnectorListener* listener)
{
    mListener = listener;
}

int RNetdConnector::startMonitoring()
{
    int sock = socket_local_client("netd",
            ANDROID_SOCKET_NAMESPACE_RESERVED,
            SOCK_STREAM);
    if (sock < 0)
    {
        LOGE("connect netd failed!");
        return -1;
    }

    mSocket = sock;
    mIsMonitoring = true;
    pthread_create(&mMonitorHandle, NULL, doMonitoring, (void*)this);
    return 0;
}

void RNetdConnector::stopMonitoring()
{
    mIsMonitoring = false;
    pthread_join(mMonitorHandle, NULL);

    close(mSocket);
}

int RNetdConnector::doCommand(int argc, char **argv)
{
    for (int argindex = 0; argindex < argc; argindex++)
        LOGD("doCommand: %s", argv[argindex]);

    if (!mIsMonitoring) return -1;

    char *final_cmd;
    char *conv_ptr;
    int i;

    /* Check if 1st arg is cmd sequence number */
    strtol(argv[0], &conv_ptr, 10);
    if (conv_ptr == argv[0])
        final_cmd = strdup("0 ");
    else
        final_cmd = strdup("");

    if (final_cmd == NULL)
    {
        LOGE("strdup failed: %s", strerror(errno));
        return -1;
    }

    for (i = 0; i < argc; i++)
    {
        if (index(argv[i], '"'))
        {
            LOGE("argument with embedded quotes not allowed");
            free(final_cmd);
            return -1;
        }

        bool needs_quoting = index(argv[i], ' ');
        const char *format = needs_quoting ? "%s\"%s\"%s" : "%s%s%s";
        char *tmp_final_cmd;

        if (asprintf(&tmp_final_cmd, format, final_cmd, argv[i],
                    (i == (argc - 1)) ? "" : " ") < 0)
        {
            LOGE("failed asprintf: %s", strerror(errno));
            free(final_cmd);
            return -1;
        }

        free(final_cmd);
        final_cmd = tmp_final_cmd;
    }

    //TODO: maybe sync lock write/read
    if (write(mSocket, final_cmd, strlen(final_cmd) + 1) < 0)
    {
        LOGE("write: %s", strerror(errno));
        free(final_cmd);
        return -1;
    }

    free(final_cmd);

    //TODO: maybe sync wait response event
    return 0;
}

void* RNetdConnector::doMonitoring(void* arg)
{
    RNetdConnector* connector = (RNetdConnector*)arg;
    int sock = connector->mSocket;

    char *buffer = (char*)malloc(4096);
    while (connector->mIsMonitoring)
    {
        fd_set read_fds;
        struct timeval to;
        int rc = 0;

        to.tv_sec = 1;
        to.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        if ((rc = select( +1, &read_fds, NULL, NULL, &to)) < 0)
        {
            LOGE("Error in select (%s)\n", strerror(errno));
            break;
        }
        else if (!rc)
        {
            continue;
        }
        else if (FD_ISSET(sock, &read_fds))
        {
            memset(buffer, 0, 4096);
            if ((rc = read(sock, buffer, 4096)) <= 0)
            {
                if (rc == 0)
                    LOGE("Lost connection to Netd - did it crash?\n");
                else
                    LOGE("Error reading data (%s)\n", strerror(errno));
                break;
            }

            int offset = 0;
            int i = 0;

            for (i = 0; i < rc; i++)
            {
                if (buffer[i] == '\0')
                {
                    int code;
                    char tmp[4];

                    strncpy(tmp, buffer + offset, 3);
                    tmp[3] = '\0';
                    code = atoi(tmp);
                    //TODO: callback onEvent

                    LOGD("%s\n", buffer + offset);
                    offset = i + 1;
                }
            }
        }
    }

    free(buffer);
    connector->mIsMonitoring = false;

    pthread_exit((void*)"exit netd monitor thread");
    return (void*)NULL;
}
