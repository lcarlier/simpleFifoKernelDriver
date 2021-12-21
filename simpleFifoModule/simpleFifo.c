#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/printk.h>
#include <linux/device/class.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Laurent Carlier <carlier.lau@gmail.com>");

static int simple_fifo_open(struct inode* inode, struct file* file);
static ssize_t simple_fifo_write(struct file* file, char const* buf, size_t size, loff_t* offset);
static ssize_t simple_fifo_read(struct file* file, char* buf, size_t size, loff_t* offset);
static int simple_fifo_release(struct inode* inode, struct file* file);

static const struct file_operations simpleFifo_fops = {
        .owner      = THIS_MODULE,
        .open = &simple_fifo_open,
        .write = &simple_fifo_write,
        .read = &simple_fifo_read,
        .release = &simple_fifo_release
};

#define MAX_FIFO_SIZE ((uint8_t)64)

struct simpleFifo_device_data {
    struct device *dev;
    struct cdev cdev;
    struct mutex open_file_list_mutex;
    struct list_head opened_file_list;
};

struct file_private_data {
    struct simpleFifo_device_data* parent;
    struct list_head file_entry;
    uint8_t data[MAX_FIFO_SIZE];
    uint8_t writeOffset;
    uint8_t readOffset;
    uint8_t size;
};

static int dev_major;
static struct class* my_class;
static struct simpleFifo_device_data simpleFifo_data;

static int __init simple_fifo_init(void)
{
	int err;
	dev_t devNumber;

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

    simpleFifo_data.dev = device_create(my_class, NULL, MKDEV(dev_major, 0), NULL, "simplefifo-%d", 0);
    if(simpleFifo_data.dev == NULL)
    {
        goto cdev_del;
    }

    mutex_init(&simpleFifo_data.open_file_list_mutex);
    INIT_LIST_HEAD(&simpleFifo_data.opened_file_list);

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

    struct file_private_data* fpd = devm_kzalloc(data->dev, sizeof(struct file_private_data), GFP_KERNEL);
    if(fpd == NULL)
    {
        return -ENOMEM;
    }
    mutex_lock(&data->open_file_list_mutex);
    INIT_LIST_HEAD(&fpd->file_entry);
    list_add(&fpd->file_entry, &data->opened_file_list);
    fpd->parent = data;
    file->private_data = (void*)fpd;
    fpd->readOffset = 0;
    fpd->writeOffset = 0;
    fpd->size = 0;
    mutex_unlock(&data->open_file_list_mutex);
    return 0;
}

static ssize_t simple_fifo_write(struct file* file, char const* buf, size_t size, loff_t* offset)
{
    uint8_t dataFromUser[MAX_FIFO_SIZE];
    uint8_t idx;
    uint8_t nbBytesToCopy = min(size, ((size_t)MAX_FIFO_SIZE));
    struct list_head* curListHead;
    struct simpleFifo_device_data* parent;

    struct file_private_data *writenFilePd = (struct file_private_data *) file->private_data;
    int isWrittenFileWriteOnly = (file->f_flags & O_WRONLY) != 0;
    parent = writenFilePd->parent;

    mutex_lock(&parent->open_file_list_mutex);
    list_for_each(curListHead, &parent->opened_file_list)
    {
        struct file_private_data *curFpd = list_entry(curListHead, struct file_private_data, file_entry);
        if (curFpd->size == MAX_FIFO_SIZE) {
            mutex_unlock(&parent->open_file_list_mutex);
            return 0;
        }

        if(curFpd->size + size > MAX_FIFO_SIZE)
        {
            uint8_t spaceRemaingInFifo = MAX_FIFO_SIZE - curFpd->size;
            nbBytesToCopy = min(nbBytesToCopy, spaceRemaingInFifo);
        }
        else
        {
            nbBytesToCopy = min((size_t)nbBytesToCopy, size);
        }
    }

    if(copy_from_user(&dataFromUser, buf, nbBytesToCopy))
    {
        mutex_unlock(&parent->open_file_list_mutex);
        return -EFAULT;
    }
    list_for_each(curListHead, &parent->opened_file_list)
    {
        struct file_private_data *curFpd = list_entry(curListHead, struct file_private_data, file_entry);
        if(isWrittenFileWriteOnly && (curFpd == writenFilePd))
        {
            continue;
        }
        for(idx = 0; idx < nbBytesToCopy; idx++)
        {
            curFpd->data[curFpd->writeOffset] = dataFromUser[idx];
            ++curFpd->writeOffset;
            curFpd->writeOffset %= MAX_FIFO_SIZE;
        }
        curFpd->size += nbBytesToCopy;
    }
    mutex_unlock(&parent->open_file_list_mutex);
    return nbBytesToCopy;
}

static ssize_t simple_fifo_read(struct file* file, char* buf, size_t size, loff_t* offset)
{
    struct file_private_data *fpd = (struct file_private_data*)file->private_data;
    struct simpleFifo_device_data *parent = fpd->parent;
    uint8_t dataToUser[MAX_FIFO_SIZE];
    uint8_t idx;

    mutex_lock(&parent->open_file_list_mutex);
    if(fpd->size == 0)
    {
        mutex_unlock(&parent->open_file_list_mutex);
        return 0;
    }
    size = min((size_t)fpd->size, size);
    for(idx = 0; idx < size; idx++)
    {
        dataToUser[idx] = fpd->data[fpd->readOffset];
        ++fpd->readOffset;
        fpd->readOffset %= MAX_FIFO_SIZE;
    }
    if(copy_to_user(buf, dataToUser, size))
    {
        mutex_unlock(&parent->open_file_list_mutex);
        return -EFAULT;
    }
    fpd->size -= size;
    mutex_unlock(&parent->open_file_list_mutex);
    return idx;
}

static int simple_fifo_release(struct inode* inode, struct file* file)
{
    struct file_private_data* fpd = (struct file_private_data*)file->private_data;
    struct simpleFifo_device_data* parent = fpd->parent;

    mutex_lock(&parent->open_file_list_mutex);
    list_del(&fpd->file_entry);
    devm_kfree(parent->dev, fpd);
    mutex_unlock(&parent->open_file_list_mutex);
    return 0;
};

static void __exit simple_fifo_exit(void)
{
	device_destroy(my_class, MKDEV(dev_major, 0));
    class_destroy(my_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

    printk("Simple fifo unregistered\n");
}

module_init(simple_fifo_init);
module_exit(simple_fifo_exit);
