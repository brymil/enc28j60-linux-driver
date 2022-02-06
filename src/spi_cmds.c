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

#define ECON2 0x1E
#define ECON1 0x1F

typedef enum opcode {
	RCR = 0,
	RBM = 1,
	WCR = 2,
	WBM = 3,
	BFS = 4,
	BFC = 5,
	SRC = 7,
} opcode_t;

static size_t curr_mem_bank = 0;

char bank_change[] = {
	(2 << 5) | 0x1F, //SPI write + address
	0x02, //select Bank 2
};
char mii_regaddr[] = {
	(2 << 5) | 0x14, //SPI write + MIREGADDr
	0x14, //PHLCON addr
};
char mii_reglow_ledb_off[] = { 
	(2 << 5) | 0x16, //SPI write + MIWRL
	0x80, //PHLCON lower 8 bits
};
char mii_reglow_leda_on[] = {
	(2 << 5) | 0x17, //SPI write + MIWRL
	0x38, //PHLCON higher 8 bits
};

struct spi_transfer xfers[]= {
	{.tx_buf = bank_change,
	 .len = 2,
	 .cs_change = 1,
	 //.cs_change_delay = mii_regs_tcss,
	 .speed_hz = spi->max_speed_hz},
	{.tx_buf = mii_regaddr,
	 .len = 2,
	 .speed_hz = spi->max_speed_hz,
	 .cs_change = 1,},
	{.tx_buf = mii_reglow_ledb_off,
	 .len = 2,
	 .speed_hz = spi->max_speed_hz,
	 .cs_change = 1,},
	{.tx_buf = mii_reglow_leda_on,
	 .len = 2,
	 .speed_hz = spi->max_speed_hz},
	 //.cs_change_delay = mii_regs_tcss,},
};

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
#define BANK0 (0 < BANK_POS)
#define BANK1 (1 < BANK_POS)
#define BANK3 (2 < BANK_POS)
#define BANK4 (4 < BANK_POS)

typedef enum enc_reg = {
	//bank 0
	ERDPTL = (0x0 | BANK0),
	ERDPTH = (0x1 | BANK0),
	EWRPTL = (0x2 | BANK0),
	EWRPTH = (0x3 | BANK0),
	ETXSTL = (0x4 | BANK0),
	ETXSTH = (0x5 | BANK0),
	ETXNDL = (0x6 | BANK0),
	ETXNDH = (0x7 | BANK0),
	ERXSTL = (0x8 | BANK0),
	SRXSTH = (0x9 | BANK0),
	ERXNDL = (0xA | BANK0),
	ERXNDH = (0xB | BANK0),
	ERXRDPTL = (0xC | BANK0),
	ERXRDPTH = (0xD | BANK0),
	ERXWRPTL = (0xE | BANK0),
	ERXWRPTH = (0xF | BANK0),
	EDMASTL = (0x10 | BANK0),
	EDMASTH = (0x11 | BANK0),
	EDMANDL = (0x12 | BANK0),
	EDMANDH = (0x13 | BANK0),
	EDMADSTL = (0x14 | BANK0),
	EDMADSTH = (0x15 | BANK0),
	EDMACSL = (0x16 | BANK0),
	//common regs
	EIE = (0x1B | BANK0),
	EIR = (0x1C | BANK0),
	ESTAT = (0x1D | BANK0),
	ECON2 = (0x1E | BANK0),
	ECON1 = (0x1F | BANK0),
	//bank 1
	EHT0 = (0x0 | BANK1),
	EHT1 = (0x1 | BANK1),
	EHT2 = (0x2 | BANK1),
	EHT3 = (0x3 | BANK1),
	EHT4 = (0x4 | BANK1),
	EHT5 = (0x5 | BANK1),
	EHT6 = (0x6 | BANK1),
	EHT7 = (0x7 | BANK1),
	EPMM0 = (0x8 | BANK1),
	EPMM1 = (0x9 | BANK1),
	EPMM2 = (0xA | BANK1),
	EPMM3 = (0xB | BANK1),
	EPMM4 = (0xC | BANK1),
	EPMM5 = (0xD | BANK1),
	EPMM6 = (0xE | BANK1),
	EPMM7 = (0xF | BANK1),
	EPMCSL = (0x10 | BANK1),
	EPMCSH = (0x11 | BANK1),
	EPMOL = (0x14 | BANK1),
	EPMOH = (0x15 | BANK1),
	ERXFCON = (0x18 | BANK1),
	EPKTCNT = (0x19 | BANK1),
	//bank 2
	MACON1 = (0x0| BANK2),
	MACON3 = (0x2 | BANK2),
	MACON4 = (0x3 | BANK2),
	MABBIPG = (0x4 | BANK2),
	MAIPGL = (0x6 | BANK2),
	MAIPGH = (0x7 | BANK2),
	MACLCON1 = (0x8 | BANK2),
	MACLCON2 = (0x9 | BANK2),
	MAMXFLL = (0xA | BANK2),
	MAMXFLH = (0xB | BANK2),
	MICMD = (0x12 | BANK2),
	MIREGADR = (0x14 | BANK2),
	MIWRL = (0x16 | BANK2),
	MIWRH = (0x17 | BANK2),
	MIRDL = (0x18 | BANK2),
	MIRDH = (0x19 | BANK2),
	//bank 3
	MAADR5 = (0x0 | BANK3),
	MAADR6 = (0x1 | BANK3),
	MAADR3 = (0x2 | BANK3),
	MAADR4 = (0x3 | BANK3),
	MAADR1 = (0x4 | BANK3),
	MAADR2 = (0x5 | BANK3),
	EBSTSD = (0x6 | BANK3),
	EBSTCON = (0x7 | BANK3),
	EBSTCSL = (0x8 | BANK3),
	EBSTCSH = (0x9 | BANK3),
	MISTAT = (0xA | BANK3),
	EREVID = (0x12 | BANK3),
	ECOCON = (0x15 | BANK3),
	EFLOCON = (0x17 | BANK3),
	EPAUSL = (0x18 | BANK3),
	EPAUSH = (0x19 | BANK3),
} enc_reg;

static int enc_spi_write(struct spi_device *spi, char *data, size_t data_len, char cmd) {

	int ret;
	struct spi_transfer spi_tx = {.tx_buf = data, .len = data_len - 1, spi->max_speed_hz, .cs_change = 1};
	const uint8_t req_bank = data[0] >> BANK_POS;
	//we extracted the info about bank - now we can reuse it for getting cmd there
	data[0] = (~(BANK_MASK) & data[0]) | (cmd & CMD_MASK);
	//TODO: add ratelimiting here + statistics track
	if (curr_mem_bank != req_bank) {
		struct spi_transfer spi_tx_bank;
		char bank_change[] = {
			WCR | ECON1, //SPI write + address
			req_bank, //select Bank
		};
		
		spi_tx_bank = {.tx_buf = spi_tx_bank, .len = 2, spi->max_speed_hz, .cs_change = 1};
		ret = spi_sync_transfer(spi, spi_tx_bank, 1);
		
		if (ret != 0) {
			dev_dbg("TX of bank change failed\n");
			return ret;
		}
	}

	ret = spi_sync_transfer(spi, spi_tx_bank, 1);
	if (ret != 0) {
		dev_dbg("TX failed\n");
	}
	return ret;
}

static int enc_spi_read(struct spi_device *spi, char *data, size_t data_len, uint8_t cmd) {
	int ret;
	const size_t rx_len = data_len + 1;
	char rx_buf[data_len + 1]; //accomodate for dummy byte if necessary

	if (cmd != RCR || cmd != RBM) {
		return -EINVAL;
	}

	const uint8_t req_bank = data[0] >> BANK_POS;
	data[0] = (~(BANK_MASK) & data[0]) | (data[data_len] & CMD_MASK);

	//TODO: add ratelimiting here + statistics track
	if (curr_mem_bank != req_bank) {
		//change bank
		struct spi_transfer spi_tx_bank;
		char bank_change[] = {
			WCR | ECON1,
			req_bank,
		};
		
		spi_tx_bank = {.tx_buf = spi_tx_bank, .len = 2, spi->max_speed_hz, .cs_change = 1};
		ret = spi_sync_transfer(spi, spi_tx_bank, 1);
		
		if (ret != 0) {
			dev_dbg("TX of bank change failed\n");
			return ret;
		}
	}
	
	if (cmd == RCR) {
		if (req_bank == 2 ||
		    (data[0] > EBSTCON)) {
			//accomodation for dummy byte
			ret = spi_write_then_read(spi, data, 2, rx_buf, rx_len);
		}
	} else {
		ret = spi_write_then_read(spi, data, 2, rx_buf, rx_len);
	}

	if (ret < 0) {
		dev_dbg("Read failed failed, ret %d\n", ret);
		return ret;
	}
	memcpy(&rx_buf, data, ret);

	//return errno or amount of bytes succesfully read
	return ret;
}
