#ifndef __Rstorage_Manager_h_
#define __Rstorage_Manager_h_

#include <utils/Mutex.h>
#include <utils/RefBase.h>

using namespace android;

namespace android{
class IMountService;
};

class RStorageManager
{
public:
    static RStorageManager* instance();
    static void release();

    bool isTFCardMounted();
private:
    RStorageManager();
    ~RStorageManager();

    bool initialize();

    Mutex mMutex;

    sp<IMountService> mMountService;
    static RStorageManager* mManager;
};

#endif
