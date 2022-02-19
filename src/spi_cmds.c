#include <linux/types.h>
#include <linux/spi/spi.h>
#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/mutex.h>
#include <linux/completion.h>

#include "enc28j60.h"
#include "spi_cmds.h"

#define REG_ADDR(x) (~(BANK_MASK) & x) 

//TODO: move to priv struct, protect with mutex/spinlock
static size_t curr_mem_bank = 0;
//TODO: implement for phy read
static struct hrtimer phy_timer;
const unsigned long delay = 11000;
DEFINE_MUTEX(phy_mutex);
static int max_retry = 3;
DECLARE_COMPLETION(phy_busy_done);

static enum hrtimer_restart phy_wait_callback(struct hrtimer *timer) {
	complete(&phy_busy_done);
	return HRTIMER_NORESTART;
}

static void phy_wait(void) {	
	hrtimer_init(&phy_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED);
	phy_timer.function = phy_wait_callback;

	const ktime_t interval = ktime_set(0, delay); 
	hrtimer_set_expires(&phy_timer, interval);
	hrtimer_start(&phy_timer, interval, HRTIMER_MODE_ABS);
}

static int check_bank(struct spi_device *spi, char req_bank) {
	int ret;
	if (curr_mem_bank != req_bank) {
		char bank_change[] = {
			WCR | REG_ADDR(ECON1),
			req_bank,
		};

		struct spi_transfer spi_tx_bank = {
			.tx_buf = bank_change,
			.len = 2,
			.speed_hz = spi->max_speed_hz};
		
		ret = spi_sync_transfer(spi, &spi_tx_bank, 1);
		
		if (ret != 0) {
			dev_dbg(&spi->dev, "TX of bank change failed\n");
			return ret;
		}
		curr_mem_bank = req_bank;
	}
	return ret;
}

static int poll_busybit(struct spi_device *spi) {
	printk(KERN_INFO "polling busybit\n");
	
	int ret = 0;
	size_t retry = 0;

	for (;retry < max_retry;++retry) {
		char rx_buf[2];
		char mistat_busy[] = {
			RCR | REG_ADDR(MISTAT),
		};
		const int ret = spi_write_then_read(spi, mistat_busy, 1, rx_buf, 2);
		if (ret < 0) {
			dev_dbg(&spi->dev, "Failed to poll busy bit for PHY, ret %d\n", ret);
		}
		
		printk(KERN_INFO "Content from read - dummy %02x, data - %02x\n", rx_buf[0], rx_buf[1]);

		if ((rx_buf[1] & 0x01) == 0) {
			printk(KERN_INFO "busy bit is 0 !\n");
			break;
		}
		else if (retry == max_retry) {
			//TODO: raise CS pin ?
			printk(KERN_INFO "poll failed\n");
			return -ECANCELED;
		} else {
			printk(KERN_INFO " poll %d\n", retry);
			phy_wait();
			wait_for_completion(&phy_busy_done);
			break;
		}
	}
	return ret;
}

static int phy_read_access(struct spi_device *spi, char data) {
	int ret;
	char miregadr_phyaddr[] = {
		WCR | REG_ADDR(MIREGADR),
		data,
	};
	struct spi_transfer read_phyreg = {
		.tx_buf = miregadr_phyaddr,
		.len = 2,
		.speed_hz = spi->max_speed_hz,
		};

	ret = spi_sync_transfer(spi, &read_phyreg, 1);
	if (ret < 0) {
		dev_dbg(&spi->dev, "Failed to set PHY reg addr for reading, ret %d\n", ret);
		return ret;
	}
	
	char set_miird_regs[] = {
		WCR | REG_ADDR(MICMD),
		0x01,
	};
	struct spi_transfer set_miird = {
		.tx_buf = set_miird_regs,
		.len = 2,
		.speed_hz = spi->max_speed_hz,
	};

	ret = spi_sync_transfer(spi, &set_miird, 1);
	if (ret < 0) {
		dev_dbg(&spi->dev, "Failed to set MIIRD, ret %d\n", ret);
		return ret;
	}

	ret = poll_busybit(spi);
	if (ret < 0) {
		return ret;
	}
	
	char clear_mistat_busy[] = {
		WCR | REG_ADDR(MICMD),
		0,
	};

	struct spi_transfer clear_mistat_mird = {
		.tx_buf = clear_mistat_busy,
		.len = 2,
		.speed_hz = spi->max_speed_hz,
		};

	ret = spi_sync_transfer(spi, &clear_mistat_mird, 1);
	if (ret < 0) {
		dev_dbg(&spi->dev, "Failed to set MIIRD, ret %d\n", ret);
	}

	return ret;
}

int enc_spi_write(struct spi_device *spi, char *data, size_t data_len, char cmd) {

	int ret;
	struct spi_transfer spi_tx = {.tx_buf = data, .len = data_len, .speed_hz = spi->max_speed_hz};
	const uint8_t req_bank = data[0] >> BANK_POS;
	
	if (cmd == RCR || cmd == RBM) {
		return -EINVAL;
	}
	if (!data_len) {
		return -ENOMEM;
	}
	
	data[0] = (REG_ADDR(data[0])) | (cmd & CMD_MASK);	
	//TODO: add ratelimiting here + statistics track
	check_bank(spi, req_bank);

	if (cmd == WCR) {
		if (((req_bank == 2) && (data[0] > REG_ADDR(MAMXFLH))) ||
				data[0] == REG_ADDR(MISTAT)) {
			printk(KERN_INFO "Accessing PHY\n");
			ret = spi_sync_transfer(spi, &spi_tx, 1);
			if (ret != 0) {
				dev_dbg(&spi->dev, "PHY TX failed\n");
			}
			
			ret = mutex_lock_interruptible(&phy_mutex);
			if (ret < 0) {
				dev_dbg(&spi->dev, "Failed to get mutex ! Ret %d\n", ret);
				return ret;
			}

			ret = poll_busybit(spi);
			if (ret < 0) {
				return ret;
			}
			mutex_unlock(&phy_mutex);
		}
	} else {
		ret = spi_sync_transfer(spi, &spi_tx, 1);
		if (ret < 0) {
			dev_dbg(&spi->dev, "TX failed\n");
		}
	}

	if (ret < 0) {
		dev_dbg(&spi->dev, "TX failed\n");
	}
	return ret;
}

int enc_spi_read(struct spi_device *spi, char *data, size_t buf_size, uint8_t cmd) {
	int ret;
	if (cmd != RCR || cmd != RBM) {
		return -EINVAL;
	}
	if (!buf_size) {
		return -ENOMEM;
	}

	const uint8_t req_bank = data[0] >> BANK_POS;
	data[0] = (REG_ADDR(data[0])) | (cmd & CMD_MASK);
	//TODO: add ratelimiting here + statistics track
	check_bank(spi, req_bank);

	if (cmd == RCR) {
		if (req_bank == 2 || ((req_bank == 3) && 
			(data[0] < REG_ADDR(EBSTSD) || data[0] == REG_ADDR(MISTAT)))) {
			const size_t adapt_rx_len = 2;
			char adapt_rx_buf[adapt_rx_len]; 
			if ((data[0] >= REG_ADDR(MICMD)) || data[0] == REG_ADDR(MISTAT)) {
				const size_t phy_rx_len = 3;
				char phy_rx_buf[phy_rx_len];
				char read_phy_register[] = {
					RCR | REG_ADDR(MIRDL),
					RCR | REG_ADDR(MIRDH)};
				
				ret = mutex_lock_interruptible(&phy_mutex);
				if (ret < 0) {
					dev_dbg(&spi->dev, "Failed to get mutex !\n ret %d", ret);
					return ret;
				}

				ret = phy_read_access(spi, data[0]);
				if (ret < 0) {
					return ret;
				}
				mutex_unlock(&phy_mutex);

				ret = spi_write_then_read(spi, read_phy_register, 2, phy_rx_buf,
						phy_rx_len);
				if (ret < 0) {
					dev_dbg(&spi->dev, "PHY Read failed failed, ret %d\n", ret);
					return ret;
				}
				data[0] = phy_rx_buf[1];
				data[1] = phy_rx_buf[2];
			} else {
				ret = spi_write_then_read(spi, data, 1, adapt_rx_buf, adapt_rx_len);
				data[0] = adapt_rx_buf[1];
			}
		}
	} else {
		ret = spi_write_then_read(spi, data, 1, data, buf_size);
	}

	if (ret < 0) {
		dev_dbg(&spi->dev, "Read failed failed, ret %d\n", ret);
	}

	return ret;
}
