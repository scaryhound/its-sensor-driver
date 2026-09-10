#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/random.h>

#define SENSOR_ID 101

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ITS Edge Gateway Developer");
MODULE_DESCRIPTION("Simulated ITS Telemetry Sensor Driver");

static int sensor_open(struct inode *inode, struct file *file) {
    pr_info("its_sensor: Device opened\n");
    return 0;
}

static int sensor_release(struct inode *inode, struct file *file) {
    pr_info("its_sensor: Device closed\n");
    return 0;
}

static ssize_t sensor_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char telemetry_data[64];
    unsigned int random_speed;
    int len;

    /* If the file position is greater than 0, we've already output the string. Return EOF. */
    if (*ppos > 0) {
        return 0;
    }

    /* Generate a random speed between 30 and 90 */
    get_random_bytes(&random_speed, sizeof(random_speed));
    random_speed = (random_speed % 61) + 30; 

    /* Format the simulated telemetry string */
    len = snprintf(telemetry_data, sizeof(telemetry_data), "ID:%d,SPEED:%u\n", SENSOR_ID, random_speed);

    /* Copy the data securely to user space */
    if (copy_to_user(buf, telemetry_data, len)) {
        return -EFAULT;
    }

    *ppos += len;
    return len;
}

static const struct file_operations sensor_fops = {
    .owner   = THIS_MODULE,
    .open    = sensor_open,
    .read    = sensor_read,
    .release = sensor_release,
};

static struct miscdevice sensor_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "its_sensor",
    .fops  = &sensor_fops,
};

static int __init its_sensor_init(void) {
    int ret;
    ret = misc_register(&sensor_miscdev);
    if (ret) {
        pr_err("its_sensor: Failed to register misc device\n");
        return ret;
    }
    pr_info("its_sensor: Module loaded successfully. Node created at /dev/its_sensor\n");
    return 0;
}

static void __exit its_sensor_exit(void) {
    misc_deregister(&sensor_miscdev);
    pr_info("its_sensor: Module unloaded\n");
}

module_init(its_sensor_init);
module_exit(its_sensor_exit);
