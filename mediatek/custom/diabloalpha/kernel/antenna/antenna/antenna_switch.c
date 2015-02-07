/* linux/drivers/antenna/antenna_switch.c
 *
 * ChangeLog
 *
 * Author Kong,Troy 2013-04-08: <yiliang.kong@tcl.com>
 * Copyright 2013 TCL. All Rights Reserved.
 *	-initial version.
 *
 * Used GPIO150 as the SPDT control logic for switch antenna direction.
 * output H，low band antenna switch to bottom.
 * output L，low band antenna switch to top.
 * sys node enable, start/stop SPDT control logic.
 * sys node interval, set SPDT switch interval time.
 * sys node select, select RF antenna bottom or top.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/sched.h>

#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

static int debug = 0;
#define	ANT_DBG(format, arg...) do { if (debug) printk(KERN_INFO "[ANTENNA/Switch] " format "\n"  , ## arg); } while (0)

struct antenna_switch_control {
	struct timer_list switch_timer;
	unsigned int switch_gpio;
	unsigned int state;
	unsigned int enable;
	unsigned int interval;
};

static ssize_t antenna_switch_show_select(struct device *dev, struct device_attribute *attr, char *buf)
{	
	struct antenna_switch_control *antenna = (struct antenna_switch_control*)dev_get_drvdata(dev);
	
	return sprintf(buf, "%d \n", antenna->state);
}

static ssize_t antenna_switch_store_select(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct antenna_switch_control *antenna = (struct antenna_switch_control*)dev_get_drvdata(dev);
    	unsigned int value;

	/* Fetch antenna switch enable vaule */
	if (sscanf(buf, "%u", &value) != 1) {
		ANT_DBG("error: Invalid values\n");
		return -EINVAL;
	}

	/* Select to input antenna switch state */
	if (antenna->state != value)
	{
		antenna->state = value;
		if (antenna->state)
			mt_set_gpio_out(antenna->switch_gpio, GPIO_OUT_ONE);
		else
			mt_set_gpio_out(antenna->switch_gpio, GPIO_OUT_ZERO);
	}
	ANT_DBG("antenna_switch_store_select: %d \n", antenna->state);
	
	return count;
}

static ssize_t antenna_switch_show_enable(struct device *dev, struct device_attribute *attr, char *buf)
{	
	struct antenna_switch_control *antenna = (struct antenna_switch_control*)dev_get_drvdata(dev);
	
	return sprintf(buf, "%d \n", antenna->enable);
}

static ssize_t antenna_switch_store_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct antenna_switch_control *antenna = (struct antenna_switch_control*)dev_get_drvdata(dev);
    	unsigned int value;

	/* Fetch antenna switch enable vaule */
	if (sscanf(buf, "%u", &value) != 1) {
		ANT_DBG("error: Invalid values\n");
		return -EINVAL;
	}

	antenna->enable = value;
	ANT_DBG("antenna_switch_store_enable: %d \n", antenna->enable);
	
	/* Start/stop timer controllor for antenna switch logic */
	if (antenna->enable)
		mod_timer(&antenna->switch_timer, jiffies + msecs_to_jiffies(antenna->interval));
	else
		del_timer_sync(&antenna->switch_timer);	
	
	return count;
}

static ssize_t antenna_swtich_show_interval(struct device *dev, struct device_attribute *attr, char *buf)
{	
	struct antenna_switch_control *antenna = (struct antenna_switch_control*)dev_get_drvdata(dev);
	
	return sprintf(buf, "%d \n", antenna->interval);
}

static ssize_t antenna_switch_store_interval(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
   	struct antenna_switch_control *antenna = (struct antenna_switch_control*)dev_get_drvdata(dev);
    	unsigned int value;

	/* Fetch antenna switch interval vaule */
	if (sscanf(buf, "%u", &value) != 1) {
		ANT_DBG("error: Invalid values\n");
		return -EINVAL;
	}

	antenna->interval = value;
	ANT_DBG("antenna_switch_store_interval: %d \n", antenna->interval);

	return count;
}

DEVICE_ATTR(select, 0664, antenna_switch_show_select, antenna_switch_store_select);
DEVICE_ATTR(enable, 0664, antenna_switch_show_enable, antenna_switch_store_enable);
DEVICE_ATTR(interval, 0664, antenna_swtich_show_interval, antenna_switch_store_interval);

static struct device_attribute *antenna_switch_attr_list[] = {
	&dev_attr_select,	/* antenna switch seltect: 0:bottom 1:top */
	&dev_attr_enable,	/* antenna switch control power: 0: disable 1: enable */ 
	&dev_attr_interval,	/* antenna switch interval time */
};

static int antenna_switch_create_attr(struct device *dev)
{
    	int idx, err = 0;
    	int num = (int)(sizeof(antenna_switch_attr_list)/sizeof(antenna_switch_attr_list[0]));
    	
	if (!dev)
		return -EINVAL;

    	for (idx = 0; idx < num; idx++)
 	{
        	if ((err = device_create_file(dev, antenna_switch_attr_list[idx]))) 
		{            
            		ANT_DBG("device_create_file (%s) = %d\n", antenna_switch_attr_list[idx]->attr.name, err);        
            		break;
        	}
    	}
    
    	return err;
}

static int antenna_switch_delete_attr(struct device *dev)
{
    	int idx, err = 0;
    	int num = (int)(sizeof(antenna_switch_attr_list)/sizeof(antenna_switch_attr_list[0]));
    
    	if (!dev)
        	return -EINVAL;

    	for (idx = 0; idx < num; idx++) 
        	device_remove_file(dev, antenna_switch_attr_list[idx]);

    	return err;
}

static int antenna_switch_gpio_config(struct antenna_switch_control* antenna)
{
	/* Turn off LDP_VGP4 */
	hwPowerDown(MT65XX_POWER_LDO_VGP4, "RF");

	/* Configure gpio150 MUX for antenna direction switch */
	mt_set_gpio_mode(antenna->switch_gpio, GPIO_MODE_GPIO);
	mt_set_gpio_dir(antenna->switch_gpio, GPIO_DIR_OUT);
	mt_set_gpio_pull_enable(antenna->switch_gpio, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(antenna->switch_gpio, GPIO_PULL_DOWN);

#ifdef MINI_ANTENNA_REVERSE
	mt_set_gpio_out(antenna->switch_gpio, GPIO_OUT_ZERO);
	antenna->state = 0;
#else
	mt_set_gpio_out(antenna->switch_gpio, GPIO_OUT_ZERO);
	antenna->state = 0;
#endif

	/* Turn on VDD33, LDP_VGP4 */
    	hwPowerOn(MT65XX_POWER_LDO_VGP4, VOL_2500, "RF");
    	msleep(20);
	
	return 0;
}

static void antenna_switch_timer(unsigned long _arg)
{
	struct antenna_switch_control* antenna = (struct antenna_switch_control*) _arg;

	/* Antenna reverse switch gpio */	
	if (antenna->state)
		mt_set_gpio_out(antenna->switch_gpio, GPIO_OUT_ZERO);
	else
		mt_set_gpio_out(antenna->switch_gpio, GPIO_OUT_ONE);

	antenna->state = !antenna->state;
	ANT_DBG("antenna_switch_timer state: %d \n", antenna->state);

	/* Check antenna switch control status */
	if (antenna->enable)
		mod_timer(&antenna->switch_timer, jiffies + msecs_to_jiffies(antenna->interval));
}

static int antenna_switch_probe(struct platform_device *pdev)
{
	static int retval;
	struct antenna_switch_control* antenna = NULL;
	
	/* Allocation antenna switch control device region */
	antenna = kzalloc(sizeof(struct antenna_switch_control), GFP_KERNEL);
	if (!antenna)
	{
		retval = -ENOMEM;
		goto err1;
	}

	/* Initialize antenna switch control platform data */
	antenna->switch_gpio = GPIO94;
	antenna->enable = 0;
	antenna->interval = 1;
	platform_set_drvdata(pdev, antenna);

	/* Configure gpio150 for antenna switch control logic */
	if (antenna_switch_gpio_config(antenna) != 0) 
	{
		retval = -EIO;
		goto err1;
	}

	/* Setup antenna switch control timer */
	setup_timer(&antenna->switch_timer, antenna_switch_timer, (unsigned long)antenna);

	/* Create antenna switch sys attribute file */
	if (retval = antenna_switch_create_attr(&pdev->dev))
	{
		ANT_DBG("create attribute err = %d\n", retval);
		goto err2;
	}

	ANT_DBG("Antenna switch controller installed! \n");
	return 0;
err2:
	del_timer(&antenna->switch_timer);
err1:
	kfree(antenna);
	ANT_DBG("Antenna switch controller failed to install!\n");	
	
	return retval;
}

static int antenna_switch_remove(struct platform_device *pdev)
{
	static int retval;
	struct antenna_switch_control* antenna = platform_get_drvdata(pdev);

	/* Remove antenna switch device attribution file */
	if (retval = antenna_switch_delete_attr(&pdev->dev))
	{
		ANT_DBG("device_remove_file fail: %d\n", retval);
	} 

	/* Delete antenna switch control timer */
	del_timer(&antenna->switch_timer);
	kfree(antenna);
	ANT_DBG("Antenna switch controller unstalled!\n");
	
	return retval;
}

static struct platform_driver antenna_switch = {
	.driver     = {
		.owner  = THIS_MODULE,
		.name   = "antenna_switch",
	},
	.probe      = antenna_switch_probe,
	.remove     = antenna_switch_remove,
};

static struct platform_device antenna_switch_device = {
	.name = "antenna_switch",
	.id = -1
};

static int __init antenna_switch_init(void)
{
//=======================================================================
// Antenna Switch Controller
//=======================================================================
	int retval; 

	retval = platform_device_register(&antenna_switch_device);
	if (retval != 0){
		return retval;
	}

	return platform_driver_register(&antenna_switch);
}

static void __exit antenna_switch_exit(void)
{
	platform_driver_unregister(&antenna_switch);
}

module_init(antenna_switch_init);
module_exit(antenna_switch_exit);
module_param(debug, int, 0644);

MODULE_AUTHOR("Troy.Kong");
MODULE_DESCRIPTION("Antenna switch driver");
MODULE_LICENSE("GPL");
MODULE_PARM_DESC(debug, "Enable debug log");
