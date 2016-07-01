/*************************************************************************
	> File Name: serial.c
	> Author: 
	> Mail:  
	> Created Time: 2016年06月29日 星期三 16时35分03秒
 ************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termio.h>

#include "factory_test_log.h"

#include "serial.h"


static int serial_init(int fd)
{
    struct termios options;

    if (tcgetattr(fd, &options) != 0)
    {
        LOGE("get serial options failed.");
        return -1;
    }

    /* modify control flag */
    options.c_cflag |= (CLOCAL | CREAD);  /* 修改控制模式，不会占用串口，并使能接收 */
    options.c_cflag &= ~CRTSCTS;          /* 无硬件流控制 */
    options.c_cflag &= ~CSIZE;            /* 设置数据位之前屏蔽掉其它标志位 */
    options.c_cflag |= CS8;               /* 8位数据长度 others: CS5,CS6,CS7*/
    options.c_cflag &= ~CSTOPB;           /* 1位停止位 */

    options.c_cflag &= ~PARENB;           /* 无奇偶校验 */
    options.c_iflag &= ~INPCK;

    /* 奇校验
     * options.c_cflag |= (PARENB | PARODD);
     * options.c_iflag |= INPCK;
     */

    /* 偶校验
     * options.c_cflag |= PARENB;
     * options.c_cflag &= ~PARODD;
     * options.c_iflag |= INPCK;
     */

    options.c_oflag &= ~OPOST;           /* 修改输出模式，原始数据输出 */
    options.c_lflag = 0;                 /* 不激活终端模式 */

    // 设置等待时间和最小接收字符,作用于read
    //options.c_cc[VTIME] = 1;             /* 读取一个字符等待 1*(1/10)s */
    //options.c_cc[VMIN]  = 1;             /* 读取字符的最少个数为1 */

    // 设置波特率:115200KHz
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    // 溢出数据可以接受，但是不读
    tcflush(fd, TCIFLUSH);

    // 设置新属性, TCSANOW:所有改变立即生效
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        LOGE("change serial options failed.");
        return -1;
    }

    return 0;
}

int serial_open(const char *dev)
{
    // open ttyusb0
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        LOGE("open %s failed.(%s)", dev, strerror(errno));
        return -1;
    }

    // init serial
    if (serial_init(fd) != 0)
    {
        LOGE("serial init failed.");
        return -1;
    }

    LOGD("serial open success(fd=%d).", fd);
    return fd;
}

void serial_close(int fd)
{
    close(fd);
}

static int _write(int _fd, const void *data, int datalen)
{
    int len = 0;
    len = write(_fd, data, datalen);
    if (len <= datalen)
        return len;
    else
    {
        LOGD("_write error(len=%d).", len);
        tcflush(_fd, TCOFLUSH); // 清空发送缓冲区内容 
        return -1;
    }
}


int serial_read(int fd, void *data, int datalen)
{
    int len = 0;

    if (fd < 0)
        return -1;

    if (data == NULL || datalen < 0)
        return -1;

    len = read(fd, data, datalen);
    return len;
}

int serial_write(int fd, const void *data, int datalen)
{
    int len = 0;
    int tollen = 0;

    if (fd < 0)
        return -1;

    do
    {
        len = _write(fd, data + tollen, datalen - tollen);
        if (len == -1)
            return -1;
        else
        {
            tollen += len;
        }
    } while(tollen < datalen);

    return 0;
}

