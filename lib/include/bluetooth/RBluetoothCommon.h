#ifndef __RBLUETOOTH_COMMON_H_
#define __RBLUETOOTH_COMMON_H_

#include <hardware/hardware.h>
#include <hardware/bluetooth.h>

#define PRINT_ADDR(addr) LOGD("addr: %02x:%02x:%02x:%02x:%02x:%02x", \
        (addr)[0], (addr)[1], (addr)[2], (addr)[3], (addr)[4], (addr)[5]);

/** Bluetooth Address */
struct RBluetoothAddr
{
    uint8_t addr[6];
};

/** Bluetooth PinKey Code */
struct RBluetoothPinCode
{
    uint8_t pin[16];
};

#endif
