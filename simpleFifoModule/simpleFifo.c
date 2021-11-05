#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Laurent Carlier <carlier.lau@gmail.com>");

static int simple_fifo_open(struct inode* inode, struct file* file);
static ssize_t simple_fifo_write(struct file* file, char const* buf, size_t size, loff_t* offset);

static const struct file_operations simpleFifo_fops = {
        .owner      = THIS_MODULE,
        .open = &simple_fifo_open,
        .write = &simple_fifo_write
};

#define MAX_FIFO_SIZE 64

struct simpleFifo_device_data {
    struct cdev cdev;
    uint8_t data[MAX_FIFO_SIZE];
    uint8_t currentOffset;
    uint8_t size;
};

static int dev_major;
static struct class* my_class;
static struct simpleFifo_device_data simpleFifo_data;

static int __init simple_fifo_init(void)
{
	int err;
	dev_t devNumber;
    struct device* dev;

	err = alloc_chrdev_region(&devNumber, 0, 1, "simpleFifo");
    if(err < 0)
    {
        return 1;
    }

	dev_major = MAJOR(devNumber);

	my_class = class_create(THIS_MODULE, "simpleFifo");
    if(my_class == NULL)
    {
        goto unregister_chrdev_region;
    }

    cdev_init(&simpleFifo_data.cdev, &simpleFifo_fops);
    simpleFifo_data.cdev.owner = THIS_MODULE;
    err = cdev_add(&simpleFifo_data.cdev, MKDEV(dev_major, 0), 1);
    if(err < 0)
    {
        goto unregister_chrdev_region;
    }

    dev = device_create(my_class, NULL, MKDEV(dev_major, 0), NULL, "simplefifo-%d", 0);
    if(dev == NULL)
    {
        goto cdev_del;
    }

    printk("Simple fifo registered\n");

	return 0;

cdev_del:
    cdev_del(&simpleFifo_data.cdev);
unregister_chrdev_region:
    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
    return 1;
}

static int simple_fifo_open(struct inode* inode, struct file* file)
{
    struct simpleFifo_device_data *data = container_of(inode->i_cdev, struct simpleFifo_device_data, cdev);

    file->private_data = (void*)data;
    data->currentOffset = 0;
    data->size = 0;

    return 0;
}

static ssize_t simple_fifo_write(struct file* file, char const* buf, size_t size, loff_t* offset)
{
    struct simpleFifo_device_data *data = (struct simpleFifo_device_data*)file->private_data;

    char dataFromUser[MAX_FIFO_SIZE];
    uint8_t idx;
    uint8_t nbBytesToCopy;
    if(data->size == MAX_FIFO_SIZE)
    {
        return -EAGAIN;
    }
    if(data->size + size > MAX_FIFO_SIZE)
    {
        nbBytesToCopy = MAX_FIFO_SIZE - data->size;
    }
    else
    {
        nbBytesToCopy = size;
    }

    if(copy_from_user(&dataFromUser, buf, nbBytesToCopy))
    {
        return -EFAULT;
    }
    for(idx = 0; idx < nbBytesToCopy; idx++)
    {
        data->data[data->currentOffset] = dataFromUser[idx];
        ++data->currentOffset;
        data->currentOffset %= MAX_FIFO_SIZE;
    }
    data->size += nbBytesToCopy;

    return nbBytesToCopy;
}

static void __exit simple_fifo_exit(void)
{
	device_destroy(my_class, MKDEV(dev_major, 0));
    class_destroy(my_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

    printk("Simple fifo unregistered\n");
}

module_init(simple_fifo_init);
module_exit(simple_fifo_exit);
