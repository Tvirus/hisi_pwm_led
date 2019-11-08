#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/leds.h>
#include <linux/slab.h>

#include "hisi_pwm_api.h"



#define VERSION  "1.0"


#if 1
#define DEBUG(fmt, arg...)  printk("--LED-- " fmt "\n", ##arg)
#else
#define DEBUG(fmt, arg...)
#endif

#define ERROR(fmt, arg...)  printk(KERN_ERR "--LED-- " fmt "\n", ##arg)





typedef struct
{
    struct led_classdev leddev;
    u32 ch;
    u32 freq;
    u32 inv;
    u32 max_brightness;
    u32 default_brightness;
}led_data_t;


static void led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
    led_data_t *led_data;

    led_data = dev_get_drvdata(led_cdev->dev);
    if (IS_ERR_OR_NULL(led_data))
        return;

    hipwm_set_time(led_data->ch, led_data->freq, value, led_data->inv);
}


static int led_probe(struct platform_device *dev)
{
    int ret = 0;
    led_data_t *led_data;
    const char *name = dev->dev.of_node->name;


    DEBUG("init %s", name);

    led_data = kzalloc(sizeof(*led_data), GFP_KERNEL);
    if (IS_ERR_OR_NULL(led_data))
    {
        ERROR("cannot allocate memory, size: %u !", sizeof(*led_data));
        return -ENOMEM;
    }

    if (of_property_read_u32(dev->dev.of_node, "channel", &led_data->ch))
    {
        ERROR("%s failed to find channel in dts !", name);
        ret = -EINVAL;
        goto ERR;
    }
    if (of_property_read_u32(dev->dev.of_node, "freq", &led_data->freq))
    {
        DEBUG("%s failed to find freq in dts, use default 10000", name);
        led_data->freq = 10000;
    }
    if (of_property_read_u32(dev->dev.of_node, "inverse", &led_data->inv))
    {
        DEBUG("%s failed to find inverse in dts, use default 0", name);
        led_data->inv = 0;
    }
    if (of_property_read_u32(dev->dev.of_node, "max_brightness", &led_data->max_brightness))
    {
        DEBUG("%s failed to find max_brightness in dts, use default 255", name);
        led_data->max_brightness = 255;
    }
    if (of_property_read_u32(dev->dev.of_node, "default_brightness", &led_data->default_brightness))
    {
        DEBUG("%s failed to find default_brightness in dts, use default 0", name);
        led_data->default_brightness = 0;
    }

    if (hipwm_set_time(led_data->ch, led_data->freq, led_data->default_brightness, led_data->inv))
    {
        ERROR("%s init pwm failed with  ch:%u  freq:%u  inv:%u  max_brightness:%u !",
                name,
                led_data->ch,
                led_data->freq,
                led_data->inv,
                led_data->max_brightness);
        ret = -EINVAL;
        goto ERR;
    }

    led_data->leddev.name = name;
    led_data->leddev.brightness = led_data->default_brightness;
    led_data->leddev.brightness_set = led_set;
    led_data->leddev.max_brightness = led_data->max_brightness;
    ret = led_classdev_register(NULL, &led_data->leddev);
    if (ret)
    {
        ERROR("%s register led classdev failed !", name);
        goto ERR;
    }
    platform_set_drvdata(dev, led_data);
    dev_set_drvdata(led_data->leddev.dev, led_data);

    return 0;



ERR:
    kfree(led_data);
    return ret;
}
static int led_remove(struct platform_device *dev)
{
    led_data_t *led_data;

    led_data = (led_data_t*)platform_get_drvdata(dev);
    if (IS_ERR_OR_NULL(led_data))
        return -EFAULT;

    led_classdev_unregister(&led_data->leddev);
    return 0;
}

static const struct of_device_id led_match_table[] =
{
    { .compatible = "hisi_pwm_led", },
    { /* end */ },
};
static struct platform_driver led_driver =
{
    .probe    = led_probe,
    .remove   = led_remove,
    .driver = {
        .name  = "hisi_pwm_led",
        .owner = THIS_MODULE,
        .of_match_table = led_match_table,
    },
};
static int __init led_init(void)
{
    DEBUG("Driver Version: %s", VERSION);

    if (hipwm_init())
    {
        ERROR("init hipwm failed !");
        return -ENXIO;
    }
    return platform_driver_register(&led_driver);
}
static void __exit led_exit(void)
{
    DEBUG("exit");
  //hipwm_deinit(); 不能注销
    platform_driver_unregister(&led_driver);
}


late_initcall(led_init);
module_exit(led_exit);


MODULE_AUTHOR("LLL");
MODULE_DESCRIPTION("hisi pwm led driver");
MODULE_LICENSE("GPL");
