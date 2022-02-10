#ifndef __enc28j60__h__
#define __enc28j60__h__

//These are SPI opcodes:
//Read Ctrl Reg,
//Read Buff Mem,
//Write Ctrl Reg,
//Write Buff Mem,
//Bit Field Set,
//Bit Field Clear,
//System Reset Cmd (Soft)

#define CMD_MASK 0xE0
#define CMD_POS (5)
#define RCR (0 << CMD_POS)
#define RBM (1 << CMD_POS)
#define WCR (2 << CMD_POS)
#define WBM (3 << CMD_POS)
#define BFS (4 << CMD_POS)
#define BFC (5 << CMD_POS)
#define SRC (7 << CMD_POS)

#define OPCODE_POS 5
#define ADDR_MASK 0x1F

//These are ENC28J60 defines, used by all sources
// They are defined in such way so that you don't have to specify
// the bank when you are trying to write to register.

//registers will be passed by user in the commands, but from i.e. debugfs they won't know
//in which bank it is. However we can define a uint8_t in such way that 1F will be reserved for
//reg addr which user has to pass, and the rest of high bits will be for encoding bank info.
//User will use special enum values to encode this info
//  bit numbers
//  BANK | REGISTER
//  7 6 5  4 3 2 1 0

//export to the header for utilities such as debugfs, fuse, ioctl etc.
#define BANK_MASK (0xE0)
#define BANK_POS 6
#define BANK0 (0 << BANK_POS)
#define BANK1 (1 << BANK_POS)
#define BANK2 (2 << BANK_POS)
#define BANK3 (4 << BANK_POS)

// ================= Bank 0
#define ERDPTL  (0x0 | BANK0)
#define ERDPTH  (0x1 | BANK0)
#define EWRPTL  (0x2 | BANK0)
#define EWRPTH  (0x3 | BANK0)
#define ETXSTL  (0x4 | BANK0)
#define ETXSTH  (0x5 | BANK0)
#define ETXNDL  (0x6 | BANK0)
#define ETXNDH  (0x7 | BANK0)
#define ERXSTL  (0x8 | BANK0)
#define SRXSTH  (0x9 | BANK0)
#define ERXNDL  (0xA | BANK0)
#define ERXNDH  (0xB | BANK0)
#define ERXRDPTL  (0xC | BANK0)
#define ERXRDPTH  (0xD | BANK0)
#define ERXWRPTL  (0xE | BANK0)
#define ERXWRPTH  (0xF | BANK0)
#define EDMASTL  (0x10 | BANK0)
#define EDMASTH  (0x11 | BANK0)
#define EDMANDL  (0x12 | BANK0)
#define EDMANDH  (0x13 | BANK0)
#define EDMADSTL  (0x14 | BANK0)
#define EDMADSTH  (0x15 | BANK0)
#define EDMACSL  (0x16 | BANK0)
// ============================== common regs
#define EIE  (0x1B | BANK0)
#define EIR  (0x1C | BANK0)
#define ESTAT  (0x1D | BANK0)
#define ECON2  (0x1E | BANK0)
#define ECON1  (0x1F | BANK0)
// ============================= Bank 1
#define EHT0  (0x0 | BANK1)
#define EHT1  (0x1 | BANK1)
#define EHT2  (0x2 | BANK1)
#define EHT3  (0x3 | BANK1)
#define EHT4  (0x4 | BANK1)
#define EHT5  (0x5 | BANK1)
#define EHT6  (0x6 | BANK1)
#define EHT7  (0x7 | BANK1)
#define EPMM0  (0x8 | BANK1)
#define EPMM1  (0x9 | BANK1)
#define EPMM2  (0xA | BANK1)
#define EPMM3  (0xB | BANK1)
#define EPMM4  (0xC | BANK1)
#define EPMM5  (0xD | BANK1)
#define EPMM6  (0xE | BANK1)
#define EPMM7  (0xF | BANK1)
#define EPMCSL  (0x10 | BANK1)
#define EPMCSH  (0x11 | BANK1)
#define EPMOL  (0x14 | BANK1)
#define EPMOH  (0x15 | BANK1)
#define ERXFCON  (0x18 | BANK1)
#define EPKTCNT  (0x19 | BANK1)
// =============================== Bank 2
#define MACON1  (0x0| BANK2)
#define MACON3  (0x2 | BANK2)
#define MACON4  (0x3 | BANK2)
#define MABBIPG  (0x4 | BANK2)
#define MAIPGL  (0x6 | BANK2)
#define MAIPGH  (0x7 | BANK2)
#define MACLCON1  (0x8 | BANK2)
#define MACLCON2  (0x9 | BANK2)
#define MAMXFLL  (0xA | BANK2)
#define MAMXFLH  (0xB | BANK2)
#define MICMD  (0x12 | BANK2)
#define MIREGADR  (0x14 | BANK2)
#define MIWRL  (0x16 | BANK2)
#define MIWRH  (0x17 | BANK2)
#define MIRDL  (0x18 | BANK2)
#define MIRDH  (0x19 | BANK2)
// ============================= Bank 3
#define MAADR5  (0x0 | BANK3)
#define MAADR6  (0x1 | BANK3)
#define MAADR3  (0x2 | BANK3)
#define MAADR4  (0x3 | BANK3)
#define MAADR1  (0x4 | BANK3)
#define MAADR2  (0x5 | BANK3)
#define EBSTSD  (0x6 | BANK3)
#define EBSTCON  (0x7 | BANK3)
#define EBSTCSL  (0x8 | BANK3)
#define EBSTCSH  (0x9 | BANK3)
#define MISTAT  (0xA | BANK3)
#define EREVID  (0x12 | BANK3)
#define ECOCON  (0x15 | BANK3)
#define EFLOCON  (0x17 | BANK3)
#define EPAUSL  (0x18 | BANK3)
#define EPAUSH  (0x19 | BANK3)

// These are PHY registers. They are not directly addressable. Refer to docs
// to know how to address those registers.
#define PHCON1  0x00
#define PHSTAT1 0x01
#define PHID1   0x02
#define PHID2   0x03
#define PHCON2  0x04
#define PHSTAT2 0x05
#define PHIE    0x06
#define PHR     0x07
#define PHLCON  0x08

#endif
