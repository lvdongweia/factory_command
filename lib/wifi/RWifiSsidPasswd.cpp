#define LOG_TAG "RWifiSsidPasswd"

#include "RWifiSsidPasswd.h"
#include "factory_log.h"

#include <string.h>
#include <stdlib.h>

const char* RWifiSsidPasswd::NONE_SSID = "<unknown ssid>";

//helper function
static int hex2num(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}


static int hex2byte(const char *hex)
{
    int a, b;
    a = hex2num(*hex++);
    if (a < 0)
        return -1;
    b = hex2num(*hex++);
    if (b < 0)
        return -1;
    return (a << 4) | b;
}

static int hexstr2bin(const char *hex, uint8_t *buf, size_t len)
{
    size_t i;
    int a;
    const char *ipos = hex;
    uint8_t *opos = buf;

    for (i = 0; i < len; i++) {
        a = hex2byte(ipos);
        if (a < 0)
            return -1;
        *opos++ = a;
        ipos += 2;
    }
    return 0;
}

RWifiSsidPasswd::RWifiSsidPasswd()
{
    memset(mByteArray, 0, BYTE_STREAM_LEN);
    memset(mString, 0, STRING_LEN);
    memset(mSSIDFormat, 0, SSID_FORMAT_LEN);
    memset(mPwdFormat, 0, PWD_FORMAT_LEN);
}

RWifiSsidPasswd::~RWifiSsidPasswd()
{
}

const uint8_t* RWifiSsidPasswd::getBytesFromAscii(
        const char* ascii, size_t& outBytesLen)
{
    if (!ascii) return NULL;

    outBytesLen = convertToBytes(mByteArray, BYTE_STREAM_LEN, ascii);
    return mByteArray;
}

const uint8_t* RWifiSsidPasswd::getBytesFromHex(
        const char* hexStr, size_t& outBytesLen)
{
    if (!hexStr) return NULL;

    const char* ptr = hexStr;
    if (strncmp(ptr, "0x", 2) == 0 || strncmp(ptr, "0X", 2) == 0)
        ptr += 2;

    int ret = hexstr2bin(ptr, mByteArray, strlen(ptr));
    if (!ret)
    {
        outBytesLen = strlen(ptr) / 2;
        return mByteArray;
    }

    return NULL;
}

const char* RWifiSsidPasswd::getStringFromBytes(
        const uint8_t* data, size_t len)
{
    if (!data) return NULL;

    memset(mString, 0, sizeof(mString));
    convertToString(mString, STRING_LEN - 1, data, len);
    return mString;
}

const char* RWifiSsidPasswd::getSSIDFormat(const char* ssid)
{
    const char* ssid_str = getStringFromBytes((const uint8_t*)ssid, strlen(ssid));
    if (!ssid_str || strlen(ssid_str) == 0) return NULL;

    memset(mSSIDFormat, 0, sizeof(mSSIDFormat));
    snprintf(mSSIDFormat, SSID_FORMAT_LEN, "\"%s\"", ssid_str);

    return mSSIDFormat;
}

const char* RWifiSsidPasswd::getPwdFormat(const char* passwd)
{
    const char* pwd_str = getStringFromBytes((const uint8_t*)passwd, strlen(passwd));
    if (!pwd_str || strlen(pwd_str) == 0) return NULL;

    memset(mPwdFormat, 0, sizeof(mPwdFormat));
    snprintf(mPwdFormat, sizeof(mPwdFormat), "\"%s\"", pwd_str);

    return mPwdFormat;
}

//This function is equivalent to printf_decode() at src/utils/common.c in
//the supplicant
size_t RWifiSsidPasswd::convertToBytes(uint8_t* buf, size_t maxlen, const char* str)
{
    const char *pos = str;
    size_t len = 0;
    int val;

    while (*pos) {
        if (len == maxlen)
            break;
        switch (*pos) {
            case '\\':
                pos++;
                switch (*pos) {
                    case '\\':
                        buf[len++] = '\\';
                        pos++;
                        break;
                    case '"':
                        buf[len++] = '"';
                        pos++;
                        break;
                    case 'n':
                        buf[len++] = '\n';
                        pos++;
                        break;
                    case 'r':
                        buf[len++] = '\r';
                        pos++;
                        break;
                    case 't':
                        buf[len++] = '\t';
                        pos++;
                        break;
                    case 'e':
                        buf[len++] = '\e';
                        pos++;
                        break;
                    case 'x':
                        pos++;
                        val = hex2byte(pos);
                        if (val < 0) {
                            val = hex2num(*pos);
                            if (val < 0)
                                break;
                            buf[len++] = val;
                            pos++;
                        } else {
                            buf[len++] = val;
                            pos += 2;
                        }
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        val = *pos++ - '0';
                        if (*pos >= '0' && *pos <= '7')
                            val = val * 8 + (*pos++ - '0');
                        if (*pos >= '0' && *pos <= '7')
                            val = val * 8 + (*pos++ - '0');
                        buf[len++] = val;
                        break;
                    default:
                        break;
                }
                break;
            default:
                buf[len++] = *pos++;
                break;
        }
    }

    return len;
}

//This function is equivalent to printf_encode() at src/utils/common.c in
//the supplicant
void RWifiSsidPasswd::convertToString(char *txt, size_t maxlen, const uint8_t *data, size_t len)
{
    char *end = txt + maxlen;
    size_t i;

    for (i = 0; i < len; i++) {
        if (txt + 4 > end)
            break;

        switch (data[i]) {
            case '\"':
                *txt++ = '\\';
                *txt++ = '\"';
                break;
            case '\\':
                *txt++ = '\\';
                *txt++ = '\\';
                break;
            case '\e':
                *txt++ = '\\';
                *txt++ = 'e';
                break;
            case '\n':
                *txt++ = '\\';
                *txt++ = 'n';
                break;
            case '\r':
                *txt++ = '\\';
                *txt++ = 'r';
                break;
            case '\t':
                *txt++ = '\\';
                *txt++ = 't';
                break;
            default:
                if (data[i] >= 32 && data[i] <= 127) {
                    *txt++ = data[i];
                } else {
                    txt += snprintf(txt, end - txt, "\\x%02x",
                            data[i]);
                }
                break;
        }
    }

    *txt = '\0';
}
