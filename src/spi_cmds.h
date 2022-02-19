#ifndef __spi__cmds__h__
#define __spi__cmds__h__

#include <linux/types.h>
#include <linux/spi/spi.h>
#include <linux/device.h>

int enc_spi_write(struct spi_device *spi, char *data, size_t data_len, char cmd);
int enc_spi_read(struct spi_device *spi, char *data, size_t data_len, uint8_t cmd);
#endif
