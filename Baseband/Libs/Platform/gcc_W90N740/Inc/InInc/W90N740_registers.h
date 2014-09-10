#ifndef __W90N740_REGISTERS_H__
#define __W90N740_REGISTERS_H__

#define W90N740_IO_BASE		AHB_IO_BASE

#define W90N740_REG(addr)	(*(UINT32 volatile *)(addr))

#define W90N740_PDID		W90N740_REG (W90N740_IO_BASE + 0x00000000)
#define W90N740_ARBCON		W90N740_REG (W90N740_IO_BASE + 0x00000004)
#define W90N740_PLLCON		W90N740_REG (W90N740_IO_BASE + 0x00000008)
#define W90N740_CLKSEL		W90N740_REG (W90N740_IO_BASE + 0x0000000C)


#define	W90N740_GDMA_CTL0	W90N740_REG (W90N740_IO_BASE + 0x00004000)
#define	W90N740_SRCB0		W90N740_REG (W90N740_IO_BASE + 0x00004004)
#define	W90N740_DSTB0		W90N740_REG (W90N740_IO_BASE + 0x00004008)
#define	W90N740_TCNT0		W90N740_REG (W90N740_IO_BASE + 0x0000400C)
#define	W90N740_CSRC0		W90N740_REG (W90N740_IO_BASE + 0x00004010)
#define	W90N740_CDST0		W90N740_REG (W90N740_IO_BASE + 0x00004014)
#define	W90N740_CTCNT0		W90N740_REG (W90N740_IO_BASE + 0x00004018)

#define	W90N740_GDMA_CTL1	W90N740_REG (W90N740_IO_BASE + 0x00004020)
#define	W90N740_SRCB1		W90N740_REG (W90N740_IO_BASE + 0x00004024)
#define	W90N740_DSTB1		W90N740_REG (W90N740_IO_BASE + 0x00004028)
#define	W90N740_TCNT1		W90N740_REG (W90N740_IO_BASE + 0x0000402C)
#define	W90N740_CSRC1		W90N740_REG (W90N740_IO_BASE + 0x00004030)
#define	W90N740_CDST1		W90N740_REG (W90N740_IO_BASE + 0x00004034)
#define	W90N740_CTCNT1		W90N740_REG (W90N740_IO_BASE + 0x00004038)



#define W90N740_WDTINT		1
#define W90N740_nIRQ0		2
#define W90N740_nIRQ1		3
#define W90N740_nIRQ2		4
#define W90N740_nIRQ3		5
#define W90N740_UARTINT		6
#define W90N740_TINT0		7
#define W90N740_TINT1		8
#define W90N740_USBINT0		9
#define W90N740_USBINT1		10
#define W90N740_EMCTXINT0	13
#define W90N740_EMCTXINT1	14
#define W90N740_EMCRXINT0	15
#define W90N740_EMCRXINT1	16
#define W90N740_GDMAINT0	17
#define W90N740_GDMAINT1	18


#endif

