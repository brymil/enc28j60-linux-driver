#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#include <linux/of_device.h>

#include <linux/netdevice.h>

#include "enc28j60.h"
#include "spi_cmds.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BRYJAN");
MODULE_DESCRIPTION("ENC28J60 - very fast Ethernet controller, feels like it's 90s again");

typedef enum device_id {
	enc28j60,
} device_id_t;

typedef enum tx_state {
	tx_state_invalid = 0,
} tx_state_t;

typedef enum rx_state {
	rx_state_invalid = 0,
} rx_state_t;

struct enc28j60 {
	unsigned int msg_enable;
	struct spi_device *spi;
	tx_state_t tx_state;
	spinlock_t rcv_lock;
	rx_state_t rx_state;
	struct net_device *netdev;
	struct timer_list wdog;
};

static const struct of_device_id enc28j60_dt_ids[] = {
	{.compatible = "brajan"},
	{}
};

static const struct spi_device_id enc28j60_id[] = {
	{"enc28j60", enc28j60},
	{}
};
MODULE_DEVICE_TABLE(spi, enc28j60_id);

static void mynetdev_setup(struct net_device *dev) {
	ether_setup(dev);
}

static void get_dt_data(struct spi_device *spi) {
	struct property *pptr;
	int spi_clk_pol;
	
	pptr = of_find_property(spi->dev.of_node, "spi-max-frequency", NULL);
	if (!pptr) {
		dev_info(&spi->dev, "Using default platform spi frequency\n");
	} else {
		of_property_read_u32(spi->dev.of_node, "spi-max-frequency", &spi->max_speed_hz);
	}
	
	pptr = of_find_property(spi->dev.of_node, "spi-clk-pol", NULL);
	if (!pptr) {
		dev_info(&spi->dev, "Using default platform clk pol\n");
	}
	else {
		memcpy(&spi_clk_pol, pptr->value, sizeof(spi_clk_pol));
		spi->mode = spi_clk_pol & SPI_CPOL; //idle at low state
	}
}

static void led_test(struct spi_device *spi) {
	char miregaddr[] = {MIREGADR, 0x14};
	char mireglow_ledb_on[] = {MIWRL, 0x80};
	char mireghi_leda_on[] = {MIWRH, 0x38};
	
	enc_spi_write(spi, miregaddr, 2, WCR);
	enc_spi_write(spi, mireglow_ledb_on, 2, WCR);
	enc_spi_write(spi, mireghi_leda_on, 2, WCR);

	//Readout led config
	char phy_low[2]  = {MIRDL, 0};
	char phy_high[2] = {MIRDH, 0};
	enc_spi_read(spi, phy_low, 2, RCR);
	enc_spi_read(spi, phy_high, 2, RCR);
	printk(KERN_INFO "PHY LED register status %02x %02x\n", phy_high[0],
		phy_low[1]);
}

static int enc28j60_probe(struct spi_device *spi) {
	struct enc28j60 *eth_drv;
	int ret;
	get_dt_data(spi);

	ret = spi_setup(spi);
	if (ret) {
		dev_err(&spi->dev, "spi_setup has failed with ret %d\n", ret);
		return ret;
	}

	eth_drv = devm_kzalloc(&spi->dev, sizeof(struct enc28j60), GFP_KERNEL);
	if (!eth_drv) {
		dev_err(&spi->dev, "Failed to allocate memory for device\n");
		return -ENOMEM;
	}

	eth_drv->spi = spi;
	spin_lock_init(&eth_drv->rcv_lock);
	dev_set_drvdata(&spi->dev, eth_drv);

/*	eth_drv->netdev = alloc_netdev(sizeof(struct enc28j60), "myeth%d", NET_NAME_UNKNOWN, mynetdev_setup);
	if (!eth_drv->netdev) {
		dev_err(&spi->dev, "Failed to alloc netdev\n");
		return -ENOMEM;
	}

	ret = register_netdev(eth_drv->netdev);
	if (ret != 0) {
		dev_err(&spi->dev, "Failed to register netdev\n");
		free_netdev(eth_drv->netdev);
		return ret;
	}
*/
	led_test(spi);
	return ret;
}

static int enc28j60_remove(struct spi_device *spi) {
	printk(KERN_INFO "SPI remove was called\n");
	//struct enc28j60	*dev = (struct enc28j60 *)dev_get_drvdata(&spi->dev);
	//unregister_netdev(dev->netdev);
	//free_netdev(dev->netdev);
	return 0;
}


static struct spi_driver enc28j60_spi_driver = {
	.driver = {
		.name = "enc28j60",
		.of_match_table = enc28j60_dt_ids,
	},
	.probe = enc28j60_probe,
	.remove = enc28j60_remove,
	.id_table = enc28j60_id,
};

static int enc_up (struct enc28j60 *enc) {
	mod_timer(&enc->wdog, jiffies);
	printk(KERN_INFO "enc28j60 interface is up\n");

	//err = request_irq();
	netif_wake_queue(enc->netdev);
	//napi_enable(&enc->napi);
	//enable_irq ?
	return 0;
}

static int enc_open (struct net_device *netdev) {
	struct enc28j60 *eth_drv = netdev_priv(netdev);
	int err = 0;
	printk(KERN_INFO "enc28j60 interface is open\n");

	netif_carrier_off(netdev);
	err = enc_up(enc28j60);
	if (err) {
		netif_err(eth_drv, drv, eth_drv->netdev, "Cannot open interface \n");
	}
	return err;
}

static int enc_down (struct enc28j60 *eth_drv) {
	printk(KERN_INFO "enc28j60 interface is down\n");
	
	return 0;
}

static int enc_close (struct net_device *netdev) {
	struct enc28j60 *eth_drv = netdev_priv(netdev);
	int err = 0;
	printk(KERN_INFO "enc28j60 interface is closed\n");
	
	err = enc_down(eth_drv);
	return err;
}

static int enc_stop (struct net_device *netdev) {
	printk(KERN_INFO "enc28j60 interface is stopeed\n");
	
	return NETDEV_TX_OK;
}

static int enc_xmit (struct sk_buff *skb, struct net_device *netdev) {
	printk(KERN_INFO "enc28j60 interface is xmit\n");
	
	return NETDEV_TX_OK;
}

static int enc_validate_addr (struct net_device *netdev) {
	printk(KERN_INFO "enc28j60 interface is validate addr\n");
	
	return 0;
}

static void enc_set_multicast (struct net_device *netdev) {
	printk(KERN_INFO "enc28j60 interface is set mcast\n");

}

static int enc_set_mac_addr (struct net_device *netdev, void *addr) {
	printk(KERN_INFO "enc28j60 interface is set mac adr\n");
	
	return 0;
}

static int enc_eth_ioctl (struct net_device *netdev, struct ifreq *ifr, int cmd) {
	printk(KERN_INFO "enc28j60 interface is ioctl\n");
	
	return 0;
}

static void enc_tx_timeout (struct net_device *netdev, unsigned int txqueue) {
	printk(KERN_INFO "enc28j60 interface is tx_timeout\n");

}

static int enc_netpoll (struct net_device *netdev) {
	printk(KERN_INFO "enc28j60 interface is netpoll\n");
	
	return 0;
}

static int enc_set_features (struct net_device *netdev, netdev_features_t features) {
	printk(KERN_INFO "enc28j60 interface is set features\n");
	
	return 0;
}

struct net_device_ops enc28j60_ops = {
	.ndo_open = enc_open,
	.ndo_stop = enc_stop,
	.ndo_start_xmit = enc_xmit,
	.ndo_validate_addr = enc_validate_addr,
	.ndo_set_rx_mode = enc_set_multicast,
	.ndo_set_mac_address = enc_set_mac_addr,
	.ndo_do_ioctl = enc_eth_ioctl,
	.ndo_tx_timeout = enc_tx_timeout,
#ifdef ENC_NAPI
	.ndo_poll_controller = enc_netpoll,
#endif
	.ndo_set_features = enc_set_features,	
};

module_spi_driver(enc28j60_spi_driver);
