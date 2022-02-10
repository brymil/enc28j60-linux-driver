#ifndef __spi_cmds___h__
#define __spi_cmds__h__

#include <linux/types.h>
#include <linux/spi/spi.h>
#include <linux/device.h>

#include "enc28j60.h"
#include "spi_cmds.h"

//TODO: move to priv struct, protect with mutex/spinlock
static size_t curr_mem_bank = 0;

int enc_spi_write(struct spi_device *spi, char *data, size_t data_len, char cmd) {

	int ret;
	struct spi_transfer spi_tx = {.tx_buf = data, .len = data_len, spi->max_speed_hz};
	const uint8_t req_bank = data[0] >> BANK_POS;
	
	data[0] = (~(BANK_MASK) & data[0]) | (cmd & CMD_MASK);
	
	//TODO: add ratelimiting here + statistics track
	if (curr_mem_bank != req_bank) {
		char bank_change[] = {
			WCR | ECON1,
			req_bank,
		};

		struct spi_transfer spi_tx_bank = {
			.tx_buf = bank_change,
			.len = 2,
			spi->max_speed_hz};
		
		ret = spi_sync_transfer(spi, &spi_tx_bank, 1);
		
		if (ret != 0) {
			dev_dbg(&spi->dev, "TX of bank change failed\n");
			return ret;
		}
		curr_mem_bank = req_bank;
	}

	ret = spi_sync_transfer(spi, &spi_tx, 1);
	if (ret != 0) {
		dev_dbg(&spi->dev, "TX failed\n");
	}
	return ret;
}


//TODO: NOT VERIFIED YET
int enc_spi_read(struct spi_device *spi, char *data, size_t data_len, uint8_t cmd) {
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
		char bank_change[] = {
			WCR | ECON1,
			req_bank,
		};

		struct spi_transfer spi_tx_bank = {
			.tx_buf = bank_change,
			.len = 2,
			spi->max_speed_hz,
			.cs_change = 1};
		
		ret = spi_sync_transfer(spi, &spi_tx_bank, 1);
		
		if (ret != 0) {
			dev_dbg(&spi->dev, "TX of bank change failed\n");
			return ret;
		}
		curr_mem_bank = req_bank;
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
		dev_dbg(&spi->dev, "Read failed failed, ret %d\n", ret);
		return ret;
	}
	memcpy(&rx_buf, data, ret);

	//return errno or amount of bytes succesfully read
	return ret;
}

#endif
