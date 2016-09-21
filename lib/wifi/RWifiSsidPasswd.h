#ifndef __RWIFI_SSID_PASSWD_H_
#define __RWIFI_SSID_PASSWD_H_

#include <stdio.h>
#include <unistd.h>

class RWifiSsidPasswd
{
public:
    RWifiSsidPasswd();
    ~RWifiSsidPasswd();

    //decode ssid for getting
    const uint8_t* getBytesFromHex(const char* hexStr, size_t& outBytesLen);
    const uint8_t* getBytesFromAscii(const char* ascii, size_t& outBytesLen);

    //encode ssid for setting
    const char* getStringFromBytes(const uint8_t* data, size_t len);
    const char* getSSIDFormat(const char* ssid);
    const char* getPwdFormat(const char* passwd);

public:
    static const int BYTE_STREAM_LEN = 128;
    static const int STRING_LEN = 128;
    static const int SSID_FORMAT_LEN = STRING_LEN + 4;
    static const int PWD_FORMAT_LEN = 32;
    static const char* NONE_SSID;

private:

    size_t convertToBytes(uint8_t* buf, size_t maxlen, const char* str);
    void convertToString(char *txt, size_t maxlen, const uint8_t *data, size_t len);

    uint8_t mByteArray[BYTE_STREAM_LEN];
    char mString[STRING_LEN];
    char mSSIDFormat[SSID_FORMAT_LEN];
    char mPwdFormat[PWD_FORMAT_LEN];
};

#endif
