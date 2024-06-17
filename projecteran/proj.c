#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>

static int nparam = 5;
module_param(nparam, int, 0644);
dev_t dev;
int units = 2;

#define BUFFER_SIZE 4096

struct mod_priv {
    struct cdev mod_cdev;
    struct class *mod_cl;
    struct device *mod_dev;
    int open;
    char buffer[BUFFER_SIZE];
    size_t buffer_size;
    size_t read;
    size_t write;
    size_t data_size;
    wait_queue_head_t read_queue;
    wait_queue_head_t write_queue;
};

static struct mod_priv mod_priv;

static int my_open(struct inode *ino, struct file *filp)
{
 
    filp->private_data = container_of(ino->i_cdev, struct mod_priv, mod_cdev);
    struct mod_priv *fpd = filp->private_data;

    fpd->open++;
    printk(KERN_INFO "Device open count: %d\n", fpd->open);
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    struct mod_priv *fpd = file->private_data;
    fpd->open--;
    printk(KERN_INFO "Device close count: \n");

    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct mod_priv *fpd = file->private_data;
    ssize_t ret;

    if (fpd->data_size < count ) {
        if (file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        } else {
            wait_event_interruptible(fpd->read_queue, fpd->data_size >= count);
        }
    }

    

        if (fpd->read + count <= fpd->buffer_size) {
        if (copy_to_user(buf, fpd->buffer + fpd->read, count)) {
            return -EFAULT;
        }
    } else {
        size_t part1_size = fpd->buffer_size - fpd->read;
        if (copy_to_user(buf, fpd->buffer + fpd->read, part1_size)) {
            return -EFAULT;
        }
        if (copy_to_user(buf + part1_size, fpd->buffer, count - part1_size)) {
            return -EFAULT;
        }
    }

    fpd->read = (fpd->read + count) % fpd->buffer_size;
    fpd->data_size -= count;
    wake_up_interruptible(&fpd->write_queue);

    ret = count;
    printk(KERN_INFO "triv_mod: read %zu bytes\n", count);
    return ret;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct mod_priv *fpd = file->private_data;
    ssize_t ret;
    size_t  space_at_end;

   
    if (count > (fpd->buffer_size - fpd->data_size)) {
        if (file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        } else {
            wait_event_interruptible(fpd->write_queue, (fpd->buffer_size - fpd->data_size) >= count);
        }
    }

    space_at_end = fpd->buffer_size - fpd->write;

    if (count > space_at_end) {
        if (copy_from_user(fpd->buffer + fpd->write, buf, space_at_end)) {
            return -EFAULT;
        }
        if (copy_from_user(fpd->buffer, buf + space_at_end, count - space_at_end)) {
            return -EFAULT;
        }
    } else {
        if (copy_from_user(fpd->buffer + fpd->write, buf, count)) {
            return -EFAULT;
        }
    }

    fpd->write = (fpd->write + count) % fpd->buffer_size;
    fpd->data_size += count;
    wake_up_interruptible(&fpd->read_queue);

    ret = count;
    printk(KERN_INFO "triv_mod: wrote %zu bytes\n", count);
    return ret;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

static int __init trivmod_init(void)
{
    int err;
    printk(KERN_INFO "triv_mod loaded\n");

    err = alloc_chrdev_region(&dev, 0, units, "CourseDev");
    if (err < 0)
        goto dvr_err;

    cdev_init(&mod_priv.mod_cdev, &fops);
    err = cdev_add(&mod_priv.mod_cdev, dev, units);
    if (err < 0)
        goto cdev_err;

    mod_priv.mod_cl = class_create("CourseDev");
    if (IS_ERR(mod_priv.mod_cl)) {
        err = PTR_ERR(mod_priv.mod_cl);
        goto clc_err;
    }

    mod_priv.mod_dev = device_create(mod_priv.mod_cl, NULL, dev, NULL, "myDev");
    if (IS_ERR(mod_priv.mod_dev)) {
        err = PTR_ERR(mod_priv.mod_dev);
        goto dvc_err;
    }
    mod_priv.buffer_size = BUFFER_SIZE;
    mod_priv.read = 0;
    mod_priv.write = 0;
    mod_priv.data_size = 0;

    init_waitqueue_head(&mod_priv.read_queue);
    init_waitqueue_head(&mod_priv.write_queue);

    return 0;

dvc_err:
    class_destroy(mod_priv.mod_cl);
clc_err:
    cdev_del(&mod_priv.mod_cdev);
cdev_err:
    unregister_chrdev_region(dev, units);
dvr_err:
    return err;
}

static void __exit trivmod_exit(void)
{
    device_destroy(mod_priv.mod_cl, dev);
    class_destroy(mod_priv.mod_cl);
    cdev_del(&mod_priv.mod_cdev);
    unregister_chrdev_region(dev, units);
    printk(KERN_INFO "triv_mod unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_AUTHOR("Instructor");
MODULE_PARM_DESC(nparam, "A numeric demonstration parameter");
MODULE_DESCRIPTION("A trivial exercise module");

module_init(trivmod_init);
module_exit(trivmod_exit);
