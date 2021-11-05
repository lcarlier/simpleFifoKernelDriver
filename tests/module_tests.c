#define module_init(initfn)
#define module_exit(initfn)
#define __init
#define __exit
static struct module{} __this_module;
#define THIS_MODULE (&__this_module)
/*
 * Including stdint before include simpleFifo.c makes sure that stdint types like uint8_t are available in
 * simpleFifo.c. Normally, those types are coming from the linux kernel include linux/types.h but mocking
 * this files will not generate the stdint types because no function parameter or return type is actually using those.
 */
#include <stdint.h>
#include "../simpleFifoModule/simpleFifo.c"

#include <easyMock.h>

#include <stdio.h>
#include <string.h>

static dev_t major_minor_to_test = MKDEV(42, 0);

static int cmp_not_null_pointer(const void *currentCall_ptr, const void *not_used, const char *paramName,
                       char *errorMessage) {

    if (currentCall_ptr != NULL) { return 0; }
    snprintf(errorMessage, 256, "Parameter '%s' has value '%p', was expecting non NULL pointer", paramName,
             currentCall_ptr);
    return -1;
}

/*
 * Some static helper functions to avoid retyping all the time the same in the actual tests.
 */
static void expect_alloc_chrdev_region_ok()
{
    alloc_chrdev_region_ExpectReturnAndOutput(NULL, 0, 1, "simpleFifo", 0, NULL, cmp_int, cmp_int, cmp_str, &major_minor_to_test);
}

static void expect_device_destroy(struct class* ptr_to_check)
{
    device_destroy_ExpectAndReturn(ptr_to_check, major_minor_to_test, cmp_pointer, cmp_int);
}

static void expect_unregister_chrdev_region()
{
    unregister_chrdev_region_ExpectAndReturn(major_minor_to_test, MINORMASK, cmp_int, cmp_int);
}

static void expect_class_create_ok(struct class* classToReturn)
{
    __class_create_ExpectAndReturn(THIS_MODULE, "simpleFifo", NULL, classToReturn, cmp_pointer, cmp_str, NULL);
}

static void expect_cdev_init_ok()
{
    cdev_init_ExpectAndReturn(&simpleFifo_data.cdev, &simpleFifo_fops, cmp_pointer, cmp_pointer);
}

static void expect_cdev_del(struct cdev* cdev_to_expect)
{
    cdev_del_ExpectAndReturn(cdev_to_expect, cmp_deref_ptr_struct_cdev);
}

static void expect_cdev_add_ok(struct cdev* expectedCdev)
{
    memcpy(expectedCdev, &simpleFifo_data.cdev, sizeof(struct cdev));
    expectedCdev->owner = THIS_MODULE;
    cdev_add_ExpectAndReturn(expectedCdev, major_minor_to_test, 1, 0, cmp_deref_ptr_struct_cdev, cmp_int, cmp_u_int);
}

static void expect_device_create_ok(struct class* classArg)
{
    struct device dev;
    device_create_ExpectAndReturn(classArg, NULL, major_minor_to_test, NULL, "simplefifo-%d", &dev, cmp_deref_ptr_struct_class, cmp_pointer, cmp_int, cmp_pointer, cmp_str);
}

/*
 * In all the tests, not only it is checked that the correct functions are called in order as expected, but also
 * the value to the function's parameters are checked to be correct by EasyMock
 */

int test_init_module_no_errors()
{
    // Test setup
    {
        expect_alloc_chrdev_region_ok();

        struct class classToReturn;
        expect_class_create_ok(&classToReturn);

        expect_cdev_init_ok();

        struct cdev expectedCdev;
        expect_cdev_add_ok(&expectedCdev);

        expect_device_create_ok(&classToReturn);

        printk_ExpectAndReturn(NULL, 0, NULL);
    }

    // Run function to test and check result
    {
        int rv = simple_fifo_init();

        if (rv != 0) {
            easyMock_addError(easyMock_true, "simple_fifo_init didn't return 0");
            return 1;
        }
    }
    return 0;
}

int test_init_module_alloc_chrdev_region_fail()
{
    // Test setup
    {
        //Configure alloc_chrdev_region to return -1
        alloc_chrdev_region_ExpectReturnAndOutput(NULL, 0, 1, "simpleFifo", -1, NULL, cmp_int, cmp_int, cmp_str,
                                                  &major_minor_to_test);
    }

    // Run function to test and check result
    {
        int rv = simple_fifo_init();

        if (rv == 0) {
            easyMock_addError(easyMock_true, "simple_fifo_init didn't return an error");
            return 1;
        }
    }
    return 0;
}

int test_init_module_class_create_fail()
{
    // Test setup
    {
        expect_alloc_chrdev_region_ok();

        //Configure class_create to return NULL ptr
        __class_create_ExpectAndReturn(THIS_MODULE, "simpleFifo", NULL, NULL, cmp_pointer, cmp_str, NULL);

        //Checks simple_fifo_init cleans up the previously created chrdev_region
        expect_unregister_chrdev_region();
    }

    // Run function to test and check result
    {
        int rv = simple_fifo_init();
        if (rv == 0) {
            easyMock_addError(easyMock_true, "simple_fifo_init didn't return an error");
            return 1;
        }
    }
    return 0;
}

int test_init_module_cdev_add_fail()
{
    // Test setup
    {
        expect_alloc_chrdev_region_ok();

        struct class classToReturn;
        expect_class_create_ok(&classToReturn);

        expect_cdev_init_ok();

        //Configure cdev_add to return -1
        struct cdev expectedCdev;
        memcpy(&expectedCdev, &simpleFifo_data.cdev, sizeof(expectedCdev));
        expectedCdev.owner = THIS_MODULE;
        cdev_add_ExpectAndReturn(&expectedCdev, major_minor_to_test, 1, -1, cmp_deref_ptr_struct_cdev, cmp_int,
                                 cmp_u_int);

        //Checks simple_fifo_init cleans up the previously created chrdev_region
        expect_unregister_chrdev_region();
    }

    // Run function to test and check result
    {
        int rv = simple_fifo_init();
        if (rv == 0) {
            easyMock_addError(easyMock_true, "simple_fifo_init didn't return an error");
            return 1;
        }
    }
    return 0;
}

int test_init_module_device_create_fail()
{
    // Test setup
    {
        expect_alloc_chrdev_region_ok();

        struct class classToReturn;
        expect_class_create_ok(&classToReturn);

        expect_cdev_init_ok();

        struct cdev expectedCdev;
        expect_cdev_add_ok(&expectedCdev);

        //Configure device_create to return NULL
        device_create_ExpectAndReturn(&classToReturn, NULL, major_minor_to_test, NULL, "simplefifo-%d", NULL,
                                      cmp_pointer, cmp_pointer, cmp_int, cmp_pointer, cmp_str);

        //Checks simple_fifo_init cleans up the previously created cdev
        expect_cdev_del(&expectedCdev);
        //Checks simple_fifo_init cleans up the previously created chrdev_region
        expect_unregister_chrdev_region();
    }

    // Run function to test and check result
    {
        int rv = simple_fifo_init();
        if (rv == 0) {
            easyMock_addError(easyMock_true, "simple_fifo_init didn't return an error");
            return 1;
        }
    }
    return 0;
}

int test_simple_fifo_open()
{
    struct inode inode;
    struct file file;
    struct simpleFifo_device_data data;
    data.size = 0xff;
    data.currentOffset = 0xff;
    inode.i_cdev = &data.cdev;

    int rv = simple_fifo_open(&inode, &file);
    if(rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_open didn't return 0");
    }
    if(file.private_data != &data)
    {
        easyMock_addError(easyMock_true, "simple_fifo_open didn't set file.private_data correctly. Expected: %p, got: %p", &data, file.private_data);
        return 1;
    }
    struct simpleFifo_device_data *devData = (struct simpleFifo_device_data*)file.private_data;
    if(devData->currentOffset != 0)
    {
        easyMock_addError(easyMock_true, "currentOffset hasn't been zeroized");
    }
    if(devData->size != 0)
    {
        easyMock_addError(easyMock_true, "size hasn't been zeroized");
    }
    return 0;
}

static void check_write(struct simpleFifo_device_data* data, uint8_t expectedSize, uint8_t expectedOffset, void* bufToExpect)
{
    if(data->size != expectedSize)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't update size to expectedSize (%d != %d)", data->size, expectedSize);
    }
    if(data->currentOffset != expectedOffset)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't update currentOffset to expectedOffset (%d != %d)", data->currentOffset, expectedOffset);
    }
    if(memcmp(data->data, bufToExpect, MAX_FIFO_SIZE) != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't update data.data correctly. data.data != bufToExpect");
    }
}

int test_simple_fifo_write_simple_write()
{
    struct file file;
    struct simpleFifo_device_data data = {0};
    file.private_data = (void*)&data;
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return len (%zd)", len);
    }
    check_write(&data, len, len, buf);
    return 0;
}

int test_simple_fifo_write_wrapper_write()
{
    struct file file;
    struct simpleFifo_device_data data = {0};
    data.currentOffset = MAX_FIFO_SIZE - 4;
    file.private_data = (void*)&data;
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    copy_from_user_ExpectReturnAndOutput(NULL, &buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, &buf, len);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return len (%zd)", len);
    }
    uint8_t offsetToExpect = 7;
    char bufToExpect[MAX_FIFO_SIZE] = {0};
    bufToExpect[60] = 's';
    bufToExpect[61] = 'i';
    bufToExpect[62] = 'm';
    bufToExpect[63] = 'p';
    bufToExpect[ 0] = 'l';
    bufToExpect[ 1] = 'e';
    bufToExpect[ 2] = ' ';
    bufToExpect[ 3] = 'c';
    bufToExpect[ 4] = 'h';
    bufToExpect[ 5] = 'a';
    bufToExpect[ 6] = 'r';
    check_write(&data, len, offsetToExpect, bufToExpect);
    return 0;
}

int test_simple_fifo_write_double_write()
{
    struct file file;
    struct simpleFifo_device_data data = {0};
    file.private_data = (void*)&data;
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "Call 1 of simple_fifo_write didn't return len (%zd)", len);
    }
    rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "Call 2 of simple_fifo_write didn't return len (%zd)", len);
    }
    char bufToExpect[MAX_FIFO_SIZE] = "simple charsimple char";
    check_write(&data, len*2, len*2, bufToExpect);
    return 0;
}

int test_simple_fifo_write_copy_from_user_fails()
{
    struct file file;
    struct simpleFifo_device_data data = {0};
    data.currentOffset = MAX_FIFO_SIZE - 4;
    file.private_data = (void*)&data;
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    copy_from_user_ExpectReturnAndOutput(NULL, &buf, len, 1, cmp_not_null_pointer, cmp_pointer, cmp_long, &buf, len);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != -EFAULT)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return an error");
    }
    check_write(&data, data.size, data.currentOffset, &data.data);
    return 0;
}

int test_simple_fifo_write_fifo_full()
{
    struct file file;
    struct simpleFifo_device_data data = {0};
    data.currentOffset = MAX_FIFO_SIZE - 1;
    data.size = MAX_FIFO_SIZE;
    file.private_data = (void*)&data;
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != -EAGAIN)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return an error");
    }
    check_write(&data, data.size, data.currentOffset, &data.data);
    return 0;
}

int test_simple_fifo_write_fifo_partial_write()
{
    struct file file;
    struct simpleFifo_device_data data = {0};
    data.currentOffset = 10;
    data.size = MAX_FIFO_SIZE - 4;
    file.private_data = (void*)&data;
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    copy_from_user_ExpectReturnAndOutput(NULL, buf, 4, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, 4);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != 4)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return partial write of 4 (%zd)", len);
    }
    rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != -EAGAIN)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return -EAGAIN after partial write");
    }
    uint8_t dataToExpect[MAX_FIFO_SIZE] = {0};
    dataToExpect[10] = 's';
    dataToExpect[11] = 'i';
    dataToExpect[12] = 'm';
    dataToExpect[13] = 'p';
    check_write(&data, MAX_FIFO_SIZE, 14, dataToExpect);
    return 0;
}

int test_exit_module()
{
    struct class* ptr_to_check = (struct class*)0xf00ba4;
    dev_major = 42;
    my_class = ptr_to_check;

    expect_device_destroy(ptr_to_check);

    class_destroy_ExpectAndReturn(ptr_to_check, cmp_pointer);

    expect_unregister_chrdev_region();
    printk_ExpectAndReturn(NULL, 0, NULL);

    simple_fifo_exit();

    return 0;
}

