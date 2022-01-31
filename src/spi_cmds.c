
//These are SPI opcodes:
//Read Ctrl Reg,
//Read Buff Mem,
//Write Ctrl Reg,
//Write Buff Mem,
//Bit Field Set,
//Bit Field Clear,
//System Reset Cmd (Soft)

#define OPCODE_POS 5
#define ADDR_MASK 0x1F

#define ECON2 0x1E
#define ECON1 0x1F

//Bank2
#define MICMD 0x12
#define MIREGADR 0x14
#define MIWRL 0x16
#define MIWRH 0x17
#define MIRDL 0x18
#define MIRDH 0x19

typedef enum opcode {
	RCR = 0,
	RBM = 1,
	WCR = 2,
	WBM = 3,
	BFS = 4,
	BFC = 5,
	SRC = 7,
} opcode_t;

static int send_cmd (opcode_t op, uint16_t addr, char *data, size_t data_len) {
	uint8_t op_byte;
	switch(op) {
		case WCR:
			op_byte = (WCR << OPCODE_POS) | (addr & ADDR_MASK);
			spi_write
	}
}

static int enc_spi_write() {

}

static int enc_spi_read() {

}
