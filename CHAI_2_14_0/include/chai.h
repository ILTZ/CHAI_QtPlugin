/*
*  chai.h
*  CAN Hardware Abstraction Interface
*  Version 2.14.x
*
*/

#ifndef __CHAI_H_
#define __CHAI_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CHAI_STATUS
#define CHAI_EXTRA
#define CHAI_BACKCOMPAT
#define CHAI_BETA


#ifdef WIN32

#ifndef _u8
#define _u8  UINT8
#endif
#ifndef _s8
#define _s8  INT8
#endif
#ifndef _u16
#define _u16 UINT16
#endif
#ifndef _s16
#define _s16 INT16
#endif
#ifndef _u32
#define _u32 ULONG32
#endif
#ifndef _s32
#define _s32 INT32
#endif

#ifndef _KERNEL_
#include <Windows.h>
#define can_waitobj_t HANDLE
#endif  //_KERNEL_

#include <BaseTsd.h>

#elif defined(__GNUC__)      // LINUX GCC

#ifdef __x86_64__  // asume LP64

#ifndef _u8
#define _u8  unsigned char
#endif
#ifndef _s8
#define _s8  char
#endif
#ifndef _u16
#define _u16 unsigned short
#endif
#ifndef _s16
#define _s16 short
#endif
#ifndef _u32
#define _u32 unsigned int
#endif
#ifndef _s32
#define _s32 int
#endif

#else   // x86_32

#ifndef _u8
#define _u8  unsigned char
#endif
#ifndef _s8
#define _s8  char
#endif
#ifndef _u16
#define _u16 unsigned short
#endif
#ifndef _s16
#define _s16 short
#endif
#ifndef _u32
#define _u32 unsigned long
#endif
#ifndef _s32
#define _s32 int
#endif

#endif // x86_64 or x86_32

	typedef int can_waitobj_t;

#endif // WIN32 OR LINUX


	typedef struct {
		_u32 id;
		_u8 data[8];
		_u8 len;
		_u16 flags;            /* bit 0 - RTR, 2 - EFF */
		_u32 ts;
	} canmsg_t;

	typedef struct {
		_u8 chan;
		_u8 wflags;
		_u8 rflags;
	} canwait_t;
    
#define CI_BRDSTR_SIZE 64

	typedef struct {
		_u8  brdnum;
		_u32 hwver;
		_s16 chip[4];
		char name[CI_BRDSTR_SIZE];
		char manufact[CI_BRDSTR_SIZE];
	} canboard_t;

	typedef struct {
		_u16 ewl;
		_u16 boff;
		_u16 hwovr;
		_u16 swovr;
		_u16 wtout;
	} canerrs_t;

	typedef struct {
		_s16 type;
		_s16 brdnum;
		_s32 irq;
		_u32 baddr;
		_u8 state;
		_u32 hovr_cnt;
		_u32 sovr_cnt;
		char _pad[32];
	} chipstat_t;

	typedef struct {
		_s16 type;
		_s16 brdnum;
		_s32 irq;
		_u32 baddr;
		_u8 state;
		_u32 hovr_cnt;
		_u32 sovr_cnt;
		_u8 mode;
		_u8 stat;
		_u8 inten;
		_u8 clkdiv;
		_u8 ecc;
		_u8 ewl;
		_u8 rxec;
		_u8 txec;
		_u8 rxmc;
		_u32 acode;
		_u32 amask;
		_u8 btr0;
		_u8 btr1;
		_u8 outctl;
		char _pad[8];
	} sja1000stat_t;

#define CI_CHSTAT_MAXLEN 16
#define CI_CHSTAT_STRNUM 32

	typedef struct {
		char name[CI_CHSTAT_STRNUM][CI_CHSTAT_MAXLEN];
		char val[CI_CHSTAT_STRNUM][CI_CHSTAT_MAXLEN];
	} chstat_desc_t;

#define CI_BRD_NUMS      8
#define CI_CHAN_NUMS     8
#define CIQUE_RC         0
#define CIQUE_TR         1
#define CIQUE_DEFSIZE_RC 4096
#define CIQUE_DEFSIZE_TR 64

	enum { 
		CIQUE_RC_THRESHOLD_DEF = 1, 
		CIQUE_TR_THRESHOLD_DEF = CIQUE_DEFSIZE_TR
	};

#define CI_WRITE_TIMEOUT_DEF 20
/* MAX is not used, backcompat */
#define CI_WRITE_TIMEOUT_MAX 500

/* states of CAN controller */
#define CAN_INIT      0
#define CAN_RUNNING   1
/*
*  predefined baud rates (recommended by CiA)
*  Phillips SJA1000 (16 MHz)
*/
#define BCI_1M_bt0    0x00
#define BCI_1M_bt1    0x14
#define BCI_800K_bt0  0x00
#define BCI_800K_bt1  0x16
#define BCI_500K_bt0  0x00
#define BCI_500K_bt1  0x1c
#define BCI_250K_bt0  0x01
#define BCI_250K_bt1  0x1c
#define BCI_125K_bt0  0x03
#define BCI_125K_bt1  0x1c
#define BCI_100K_bt0  0x04
#define BCI_100K_bt1  0x1c
#define BCI_50K_bt0   0x09
#define BCI_50K_bt1   0x1c
#define BCI_20K_bt0   0x18
#define BCI_20K_bt1   0x1c
#define BCI_10K_bt0   0x31
#define BCI_10K_bt1   0x1c
#define BCI_1M    BCI_1M_bt0, BCI_1M_bt1
#define BCI_800K  BCI_800K_bt0, BCI_800K_bt1
#define BCI_500K  BCI_500K_bt0, BCI_500K_bt1
#define BCI_250K  BCI_250K_bt0, BCI_250K_bt1
#define BCI_125K  BCI_125K_bt0, BCI_125K_bt1
#define BCI_100K  BCI_100K_bt0, BCI_100K_bt1
#define BCI_50K   BCI_50K_bt0, BCI_50K_bt1
#define BCI_20K   BCI_20K_bt0, BCI_20K_bt1
#define BCI_10K   BCI_10K_bt0, BCI_10K_bt1

/*
*  Error codes
*/
#define ECIOK      0            /* success */
#define ECIGEN     1            /* generic (not specified) error */
#define ECIBUSY    2            /* device or resourse busy */
#define ECIMFAULT  3            /* memory fault */
#define ECISTATE   4            /* function can't be called for chip in current state */
#define ECIINCALL  5            /* invalid call, function can't be called for this object */
#define ECIINVAL   6            /* invalid parameter */
#define ECIACCES   7            /* can not access resource */
#define ECINOSYS   8            /* function or feature not implemented */
#define ECIIO      9            /* input/output error */
#define ECINODEV   10           /* no such device or object */
#define ECIINTR    11           /* call was interrupted by event */
#define ECINORES   12           /* no resources */
#define ECITOUT    13           /* time out occured */

/*
*  Flags for CiOpen
*/
#define CIO_BLOCK   0x1         // ignored (block mode was removed in CHAI 2.x
#define CIO_CAN11   0x2
#define CIO_CAN29   0x4
/*
*  Flags for CiWaitEvent
*/
#define CI_WAIT_RC   0x1
#define CI_WAIT_TR   0x2
#define CI_WAIT_ER   0x4
/*
*  Commands for CiSetLom
*/
#define CI_LOM_OFF   0x0
#define CI_LOM_ON    0x1

	enum {
		CI_CMD_GET = 0,
		CI_CMD_SET = 1
	};

	enum {
		CI_OFF = 0,
		CI_ON = 1
	};

/*
*  Transmit status
*/
#define CI_TR_COMPLETE_OK    0x0
#define CI_TR_COMPLETE_ABORT 0x1
#define CI_TR_INCOMPLETE     0x2
#define CI_TR_DELAY          0x3

/*
*  Transmit cancel status
*/
#define CI_TRCANCEL_TRANSMITTED      0x0
#define CI_TRCANCEL_ABORTED          0x1
#define CI_TRCANCEL_NOTRANSMISSION   0x2
#define CI_TRCANCEL_DELAYABORTED     0x3

/*
* Bits in canmsg_t.flags field
*/
#define MSG_RTR  0
#define MSG_FF   2              /* if set - extended frame format is used */
#define FRAME_RTR     0x1
#define FRAME_EFF     0x4
#define FRAME_TRDELAY 0x10

/*
*  CAN-controller types 
*/
#define CHIP_UNKNOWN      0
#define SJA1000           1
#define EMU               2
#define MSCAN             3

/*
*  Manufacturers 
*/
#define MANUF_UNKNOWN      0
#define MARATHON           1
#define SA                 2
#define FREESCALE          3

/*
*  CAN adapter types 
*/
#define BRD_UNKNOWN       0
#define CAN_BUS_ISA       1
#define CAN_BUS_MICROPC   2
#define CAN_BUS_PCI       3
#define CAN_EMU           4
#define CAN2_PCI_M        5
#define MPC5200TQM        6
#define CAN_BUS_USB       7
#define CAN_BUS_PCI_E     8
#define CAN_BUS_USB_NP    9
#define CAN_BUS_USB_NPS   10
#define CHAI_VER(maj,min,sub) ( ((maj)<<16)|((min)<<8)|(sub) )
#define UNICAN_VER(maj,min,sub) ( ((maj)<<16)|((min)<<8)|(sub) )
#define VERSUB(ver) (_s32) ((ver) & 0xff)
#define VERMIN(ver) (_s32) (((ver)>>8) & 0xff)
#define VERMAJ(ver) (_s32) (((ver)>>16) & 0xff)

#ifndef _KERNEL_

#if defined WIN32 && defined CHAI_EXPORTS
#define DECLHEAD __declspec( dllexport)
#else
#ifdef WIN32
#define DECLHEAD __declspec( dllimport)
#else
#define DECLHEAD extern
#endif
#endif

	DECLHEAD _s16 CiInit(void);
	DECLHEAD _s16 CiOpen(_u8 chan, _u8 flags);
	DECLHEAD _s16 CiClose(_u8 chan);
	DECLHEAD _s16 CiStart(_u8 chan);
	DECLHEAD _s16 CiStop(_u8 chan);
	DECLHEAD _s16 CiSetFilter(_u8 chan, _u32 acode, _u32 amask);
	DECLHEAD _s16 CiSetBaud(_u8 chan, _u8 bt0, _u8 bt1);
	DECLHEAD _s16 CiTransmit(_u8 chan, canmsg_t * mbuf);
	DECLHEAD _s16 CiTrCancel(_u8 chan, _u16 * trqcnt);
	DECLHEAD _s16 CiTrStat(_u8 chan, _u16 * trqcnt);
	DECLHEAD _s16 CiRead(_u8 chan, canmsg_t * mbuf, _s16 cnt);
	DECLHEAD _s16 CiErrsGetClear(_u8 chan, canerrs_t * errs);
	DECLHEAD _s16 CiWaitEvent(canwait_t * cw, int cwcount, int tout);

#ifdef CHAI_STATUS
	DECLHEAD _s16 CiChipStat(_u8 chan, chipstat_t * stat);
	DECLHEAD _s16 CiChipStatToStr(chipstat_t * status, chstat_desc_t * desc);
	DECLHEAD _s16 CiBoardInfo(canboard_t * binfo);
	DECLHEAD _s16 CiGetFirmwareVer(_u8 chan, _u32 * ver);
	DECLHEAD _u32 CiGetLibVer(void);
	DECLHEAD _u32 CiGetDrvVer(void);
#endif

#ifdef CHAI_EXTRA
	DECLHEAD _s16 CiTrQueThreshold(_u8 chan, _s16 getset, _u16 * thres);
	DECLHEAD _s16 CiRcQueThreshold(_u8 chan, _s16 getset, _u16 * thres);
	DECLHEAD _s16 CiRcQueResize(_u8 chan, _u16 size);
	DECLHEAD _s16 CiRcQueCancel(_u8 chan, _u16 * rcqcnt);
	DECLHEAD _s16 CiRcQueGetCnt(_u8 chan, _u16 * rcqcnt);
	DECLHEAD _s16 CiBoardGetSerial(_u8 brdnum, char *sbuf, _u16 bufsize);
	DECLHEAD _s16 CiHwReset(_u8 chan);
	DECLHEAD _s16 CiSetLom(_u8 chan, _u8 mode);
	DECLHEAD void CiStrError(_s16 cierrno, char *buf, _s16 n);
	DECLHEAD void CiPerror(_s16 cierrno, const char *s);
#endif

#ifdef CHAI_BACKCOMPAT // backward compatibility: could be removed in future version
	// use CiTransmit in new code
	DECLHEAD _s16 CiWrite(_u8 chan, canmsg_t * mbuf, _s16 cnt);
	DECLHEAD _s16 CiWriteTout(_u8 chan, _s16 getset, _u16 * msec);
	// use CiSetLom in new code 
	DECLHEAD _s16 CiSJA1000SetLom(_u8 chan);
	DECLHEAD _s16 CiSJA1000ClearLom(_u8 chan);
	// use CiRcQueCancel CiRcQueResize, CiRcQueGetCnt, CiWriteTout in new code 
	DECLHEAD _s16 CiRcQueEmpty(_u8 chan);
	DECLHEAD _s16 CiQueResize(_u8 chan, _u16 size);
	DECLHEAD _s32 CiRcGetCnt(_u8 chan);
	DECLHEAD _s16 CiSetWriteTout(_u8 chan, _u16 msec);
	DECLHEAD _s16 CiGetWriteTout(_u8 chan, _u16 * msec);
#endif

#ifdef CHAI_BETA // experimental functions: could be changed or removed
	DECLHEAD can_waitobj_t * CiSysWaitObjGet(_u8 chan);
	DECLHEAD _s16 CiPnpDevListRescan(void);
	DECLHEAD _s16 CiRegRead(_u8 chan, _u32 offset, _u32 * val);
	DECLHEAD _s16 CiRegWrite(_u8 chan, _u32 offset, _u32 val);
	DECLHEAD _s16 CiTransmitSeries(_u8 chan, canmsg_t * mbuf, int cnt, int *chaierr);
#endif

	/*================================================================
	Struct canmsg_t operations
	================================================================*/
	DECLHEAD void msg_zero(canmsg_t * msg);
	DECLHEAD _s16 msg_isrtr(canmsg_t * msg);
	DECLHEAD void msg_setrtr(canmsg_t * msg);
	DECLHEAD _s16 msg_iseff(canmsg_t * msg);
	DECLHEAD void msg_seteff(canmsg_t * msg);
	DECLHEAD void msg_setdelaytr(canmsg_t * msg, _u32 mks);

#endif // no _KERNEL_

#ifdef __cplusplus
}
#endif
#endif                          /* __CHAI_H_ */
