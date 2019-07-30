#ifndef LINUX_MMC_IOCTL_H
#define LINUX_MMC_IOCTL_H
#ifdef CONFIG_MMC_CPRM_SUPPORT
#include "card.h"
#endif

#ifdef __linux__
# include "linux/types.h"
#else
# include <stdint.h>
typedef uint64_t __u64;
typedef uint32_t __u32;
typedef int32_t __s32;
typedef uint16_t __u16;
typedef uint8_t __u8;
#endif

struct mmc_ioc_cmd {
	
	int write_flag;

	
	int is_acmd;

	__u32 opcode;
	__u32 arg;
	__u32 response[4];  
	unsigned int flags;
	unsigned int blksz;
	unsigned int blocks;

	unsigned int postsleep_min_us;
	unsigned int postsleep_max_us;

	unsigned int data_timeout_ns;
	unsigned int cmd_timeout_ms;

	__u32 __pad;

	
	__u64 data_ptr;
};
#define mmc_ioc_cmd_set_data(ic, ptr) ic.data_ptr = (__u64)(unsigned long) ptr

#define MMC_IOC_CMD _IOWR(MMC_BLOCK_MAJOR, 0, struct mmc_ioc_cmd)
#ifdef CONFIG_MMC_CPRM_SUPPORT
struct mmc_ioc_cmd_extend {
	struct sd_ssr ssr;							
	u32 ccs;									
	u32 capacity_of_protected_area_in_byte;		
	u32 card_status;							
	u32 capacity;								
};
#define MMC_IOC_CMD_EXTEND _IOR(MMC_BLOCK_MAJOR, 1, struct mmc_ioc_cmd_extend)
#endif
#define MMC_IOC_MAX_BYTES  (512L * 256)
#endif

