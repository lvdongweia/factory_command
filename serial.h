/*************************************************************************
	> File Name: serial.h
	> Author: 
	> Mail:  
	> Created Time: 2016年06月29日 星期三 16时33分44秒
 ************************************************************************/

#ifndef SERIAL_H



int serial_open(const char *dev);

void serial_close(int fd);

int serial_read(int fd, void *data, int datalen);

int serial_write(int fd, const void *data, int datalen);


#endif /* SERIAL_H */
