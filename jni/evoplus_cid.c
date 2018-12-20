#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "mmc.h"

#define CID_SIZE 16
#define PROGRAM_CID_OPCODE 26
#define SAMSUNG_VENDOR_OPCODE 62

int mmc_movi_vendor_cmd(unsigned int arg, int fd) {
	int ret = 0;
	struct mmc_ioc_cmd idata = {0};

	idata.data_timeout_ns = 0x10000000;
	idata.write_flag = 1;
	idata.opcode = SAMSUNG_VENDOR_OPCODE;
	idata.arg = arg;
	idata.flags = MMC_RSP_R1B | MMC_CMD_AC;

	ret = ioctl(fd, MMC_IOC_CMD, &idata);

	return ret;
}

int cid_backdoor(int fd) {
	int ret;

	ret = mmc_movi_vendor_cmd(0xEFAC62EC, fd);
	if (ret) {
		printf("Failed to enter vendor mode. Genuine Samsung Evo Plus?\n");
	} else {
		ret = mmc_movi_vendor_cmd(0xEF50, fd);
		if (ret) {
			printf("Unlock command failed.\n");
		} else {
			ret = mmc_movi_vendor_cmd(0x00DECCEE, fd);
			if (ret) {
				printf("Failed to exit vendor mode.\n");
			}
		}
	}

	return ret;
}

int program_cid(int fd, const unsigned char *cid) {
	int ret;
	struct mmc_ioc_cmd idata = {0};

	idata.data_timeout_ns = 0x10000000;
	idata.write_flag = 1;
	idata.opcode = PROGRAM_CID_OPCODE;
	idata.arg = 0;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blksz = CID_SIZE;
	idata.blocks = 1;
	idata.data_ptr = (__u64)cid;

	ret = ioctl(fd, MMC_IOC_CMD, &idata);
	if (ret) {
		printf("Success! Remove and reinsert SD card to check new CID.\n");
	}

	return ret;
}

void show_cid(const unsigned char *cid) {
	int i;
	for (i = 0; i < CID_SIZE; i++){
		printf("%02x", cid[i]);
	}
	printf("\n");
}

unsigned char crc7(const unsigned char data[], int len) {

	int count;
	unsigned char crc = 0;

	for (count = 0; count <= len; count++) {
		unsigned char dat;
		unsigned char bits;
		if (count == len) {
			dat = 0;
			bits = 7;
		} else {
			dat = data[count];
			bits = 8;
		}
		for (; bits > 0; bits--) {
			crc = (crc << 1) + ( (dat & 0x80) ? 1 : 0 );
			if (crc & 0x80) crc ^= 0x09;
			dat <<= 1;
		}
	   crc &= 0x7f;
	}

	return ((crc << 1) + 1);
}

int parse_serial(const char *str) {

	long val;

	// accept decimal or hex, but not octal
	if ((strlen(str) > 2) && (str[0] == '0') &&
		(((str[1] == 'x')) || ((str[1] == 'X')))) {
		val = strtol(str, NULL, 16);
	} else {
		val = strtol(str, NULL, 10);
	}

	return (int)val;
}

int main(int argc, const char **argv) {
	int fd, ret, i, len;
	unsigned char cid[CID_SIZE] = {0};

	if (argc != 3 && argc != 4) {
		printf("Usage: ./evoplus_cid <device> <cid> [serial]\n");
		printf("device - sd card block device e.g. /dev/block/mmcblk1\n");
		printf("cid - new cid, must be in hex (without 0x prefix)\n");
		printf("  it can be 32 chars with checksum or 30 chars without, it will\n");
		printf("  be updated with new serial number if supplied, the checksum is\n");
		printf("  (re)calculated if not supplied or new serial applied\n");
		printf("serial - optional, can be hex (0x prefixed) or decimal\n");
		printf("  and will be applied to the supplied cid before writing\n");
		printf("\n");
		printf("Warning: use at own risk!\n");
		return -1;
	}

	len = strlen(argv[2]);
	if (len != 30 && len != 32) {
		printf("CID should be 30 or 32 chars long!\n");
		return -1;
	}

	// parse cid
	for (i = 0; i < (len/2); i++){
		ret = sscanf(&argv[2][i*2], "%2hhx", &cid[i]);
		if (!ret){
			printf("CID should be hex (without 0x prefix)!\n");
			return -1;
		}
	}

	// incorporate optional serial number
	if (argc == 4) {
		*((int*)&cid[9]) = htonl(parse_serial(argv[3]));
	}

	// calculate checksum if required
	if (len != 32 || argc == 4) {
		cid[15] = crc7(cid, 15);
	}

	// open device
	fd = open(argv[1], O_RDWR);
	if (fd < 0){
		printf("Unable to open device %s\n", argv[1]);
		return -1;
	}

	// unlock card
	//ret = 0;
	ret = cid_backdoor(fd);
	if (!ret){
		// write new cid
		printf("Writing new CID: ");
		show_cid(cid);
		ret = program_cid(fd, cid);
		if (!ret){
			printf("Success! Remove and reinsert SD card to check new CID.\n");
		}
	}
	close(fd);
	return 0;
}
