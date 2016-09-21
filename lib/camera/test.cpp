#include "camera/ActivityTestService.h"
#include "camera/pattern.h"

#include <string.h>

using namespace android;

class TouchCallback : public ActivityTestCallback
{
public:
    TouchCallback() {}

    virtual ~TouchCallback() {}

    virtual int touch(int num, const int* points)
    {
        for (int i = 0; i < num * 2; i += 2)
            printf("touch point %d: (%d, %d)\n", i / 2 + 1, points[i], points[i + 1]);

        return 0;
    }
};

static void print_usage()
{
    printf("help: help list\n\n");
    printf("start-camera: start camera activity\n");
    printf("stop-camera:  stop camera activity\n");
    printf("open-camera: open camera\n");
    printf("close-camera: close camera\n");
    printf("take-picture <filename>: take picture\n");
    printf("view-picture <filename>: view specify picture\n");
    printf("preview: camera preview\n");
    printf("compare <pattern> <filename>: test camera dirty, position, color\n\n");

    printf("start-lcd: start lcd activity\n");
    printf("stop-lcd:  stop lcd activity\n");
    printf("display <pattern>: test lcd\n\n");

    printf("start-touch: start touch activity\n");
    printf("stop-touch:  stop touch activity\n\n");

    printf("<exit>: exit\n");
}

int activity_main(int argc, char **argv)
{
    TouchCallback* callback = new TouchCallback();
    sp<ActivityTestService> service = ActivityTestService::instance();
    service->setCallback(callback);

    char cmd_str[256] = {0};
    while (1)
    {
        printf("Please Input Command:");
        memset(cmd_str, 0, 256);
        fgets(cmd_str, 256, stdin);
        if (strlen(cmd_str) == 0)
        {
            print_usage();
            continue;
        }

        if (strncmp(cmd_str, "start-camera", strlen("start-camera")) == 0)
            printf("start camera: %d\n", service->startCameraActivity());
        else if (strncmp(cmd_str, "stop-camera", strlen("stop-camera")) == 0)
            printf("stop camera: %d\n", service->stopCameraActivity());
        else if (strncmp(cmd_str, "open-camera", strlen("open-camera")) == 0)
            printf("open camera: %d\n", service->openCamera());
        else if (strncmp(cmd_str, "close-camera", strlen("close-camera")) == 0)
            printf("close camera: %d\n", service->closeCamera());
        else if (strncmp(cmd_str, "take-picture", strlen("take-picture")) == 0)
            printf("take picture: %d\n", service->takePicture(1, String16(strchr(cmd_str, ' ') + 1)));
        else if (strncmp(cmd_str, "view-picture", strlen("view-picture")) == 0)
            printf("view-picture: %d\n", service->viewPicture(1, String16(strchr(cmd_str, ' ') + 1)));
        else if (strncmp(cmd_str, "preview", strlen("preview")) == 0)
            printf("preview: %d\n", service->preview());
        else if (strncmp(cmd_str, "compare", strlen("compare")) == 0)
        {
            char* pattern_str = strchr(cmd_str, ' ') + 1;
            char* filename = strchr(pattern_str, ' ') + 1;
            printf("compare: %d\n", service->compareImage(atoi(pattern_str), 1, String16(filename)));
        }
        else if (strncmp(cmd_str, "start-lcd", strlen("start-lcd")) == 0)
            printf("start lcd: %d\n", service->startLcdActivity());
        else if (strncmp(cmd_str, "stop-lcd", strlen("stop-lcd")) == 0)
            printf("stop lcd: %d\n", service->stopLcdActivity());
        else if (strncmp(cmd_str, "display", strlen("display")) == 0)
            printf("display lcd: %d\n", service->display(atoi(strchr(cmd_str, ' ') + 1)));
        else if (strncmp(cmd_str, "start-touch", strlen("start-touch")) == 0)
            printf("start touch: %d\n", service->startTouchActivity());
        else if (strncmp(cmd_str, "stop-touch", strlen("stop-touch")) == 0)
            printf("stop touch: %d\n", service->stopTouchActivity());
        else if (strncmp(cmd_str, "exit", strlen("exit")) == 0)
            break;
        else
        {
            printf("unknown cmd\n");
            print_usage();
        }
    }

    service.clear();
    ActivityTestService::release();

    delete callback;

    return 0;
}

int main(int argc, char** argv)
{
    activity_main(argc, argv);

    printf("Continue[c] or Exit[q]:");
    while (getchar() == 'c')
    {
        activity_main(argc, argv);
        printf("Continue[c] or Exit[q]:");
    }

    return 0;
}
