#ifndef _SN_INFO_H
#define _SN_INFO_H

/* 0 ~ 31 */
struct sn_info {
	uint16_t size;
	char sn[30];
};

struct mac_addr {
	uint8_t size;
	uint8_t addr[6];
};

/* 445 ~ 450 */
struct wifi_mac_addr {
	struct mac_addr addr;
};

/* 505, 506 ~ 511 */
struct eth_mac_addr {
	struct mac_addr addr;
};

/* 498, 499 ~ 504 */
struct bt_mac_addr {
	struct mac_addr addr;
};


#define RNAND_GET_SN_INFO   _IOR('s', 0x01, struct sn_info)
#define RNAND_GET_WIFI_ADDR _IOR('s', 0x02, struct wifi_mac_addr)
#define RNAND_GET_ETH_ADDR  _IOR('s', 0x03, struct eth_mac_addr)
#define RNAND_GET_BT_ADDR   _IOR('s', 0x04, struct bt_mac_addr)

#endif
