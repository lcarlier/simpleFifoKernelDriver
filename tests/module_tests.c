#define module_init(initfn)
#define module_exit(initfn)
#define __init
#define __exit
static struct module __this_module;
#define THIS_MODULE (&__this_module)
/*
 * Including stdint before include simpleFifo.c makes sure that stdint types like uint8_t are available in
 * simpleFifo.c. Normally, those types are coming from the linux kernel include linux/types.h but mocking
 * this files will not generate the stdint types because no function parameter or return type is actually using those.
 */
#include <stdint.h>

#include "generated/uapi/linux/version.h"

#include "../simpleFifoModule/simpleFifo.c"

#include <easyMock.h>

#include <stdio.h>
#include <string.h>

static dev_t major_minor_to_test = MKDEV(42, 0);
#define DO_NOT_FAIL (0)
#define DO_FAIL (1)

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

static void expect_device_create_ok(struct class* classArg, struct device* devToReturn)
{
    device_create_ExpectAndReturn(classArg, NULL, major_minor_to_test, NULL, "simplefifo-%d", devToReturn, cmp_deref_ptr_struct_class, cmp_pointer, cmp_int, cmp_pointer, cmp_str);
}

/*
 * In all the tests, not only it is checked that the correct functions are called in order as expected, but also
 * the value to the function's parameters are checked to be correct by EasyMock
 */

int test_init_module_no_errors()
{
    // Test setup
    struct device dev;
    {
        expect_alloc_chrdev_region_ok();

        struct class classToReturn;
        expect_class_create_ok(&classToReturn);

        expect_cdev_init_ok();

        struct cdev expectedCdev;
        expect_cdev_add_ok(&expectedCdev);

        expect_device_create_ok(&classToReturn, &dev);

        __mutex_init_ExpectAndReturn(&simpleFifo_data.open_file_list_mutex, "&simpleFifo_data.open_file_list_mutex", NULL, cmp_pointer, cmp_str, NULL);
        INIT_LIST_HEAD_ExpectAndReturn(&simpleFifo_data.opened_file_list, cmp_pointer);

        _printk_ExpectAndReturn(NULL, 0, NULL);

    }

    // Run function to test and check result
    {
        int rv = simple_fifo_init();

        if (rv != 0) {
            easyMock_addError(easyMock_true, "simple_fifo_init didn't return 0");
        }
        if(simpleFifo_data.dev != &dev)
        {
            easyMock_addError(easyMock_true, "simple_fifo_init didn't set dev correctly (%p != %p)", simpleFifo_data.dev, &dev);
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
    // Test setup
    struct inode inode;
    struct file file = {0};
    struct simpleFifo_device_data data = {0};

    inode.i_cdev = &data.cdev;

    struct file_private_data pd = {0};
    pd.writeOffset = 0xca;
    pd.readOffset = 0xfe;
    pd.size = 0xde;

    devm_kzalloc_ExpectAndReturn(data.dev, sizeof(struct file_private_data), GFP_KERNEL, &pd, cmp_pointer, cmp_int, cmp_int);
    mutex_lock_ExpectAndReturn(&data.open_file_list_mutex, cmp_pointer);
    INIT_LIST_HEAD_ExpectAndReturn(&pd.file_entry, cmp_pointer);
    list_add_ExpectAndReturn(&pd.file_entry, &data.opened_file_list, cmp_pointer, cmp_pointer);
    mutex_unlock_ExpectAndReturn(&data.open_file_list_mutex, cmp_pointer);

    // Call function to test
    int rv = simple_fifo_open(&inode, &file);

    // Check results
    if(rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_open didn't return 0");
    }
    if(file.private_data != &pd)
    {
        easyMock_addError(easyMock_true, "simple_fifo_open didn't set file.private_data correctly. Expected: %p, got: %p", &data, file.private_data);
        return 1;
    }
    struct file_private_data *filePrivateData = (struct file_private_data*)file.private_data;

    struct simpleFifo_device_data *parentToExpect = &data;
    uint8_t sizeToExpect = 0;
    uint8_t readOffsetToExpect = 0;
    uint8_t writeOffsetToExpect = 0;
    if(filePrivateData->parent != parentToExpect)
    {
        easyMock_addError(easyMock_true, "parent hasn't  been set correctly modified (%p != %p)", filePrivateData->parent, parentToExpect);
    }
    if(filePrivateData->readOffset != readOffsetToExpect)
    {
        easyMock_addError(easyMock_true, "readOffset hasn't been zeroized (%d != %d)", filePrivateData->readOffset, readOffsetToExpect);
    }
    if(filePrivateData->writeOffset != writeOffsetToExpect)
    {
        easyMock_addError(easyMock_true, "writeOffset hasn't been zeroized (%d != %d)", filePrivateData->writeOffset, writeOffsetToExpect);
    }
    if(filePrivateData->size != sizeToExpect)
    {
        easyMock_addError(easyMock_true, "size hasn't been zeroized (%d != %d)", filePrivateData->size, sizeToExpect);
    }
    return 0;
}

int test_simple_fifo_open_devm_kzalloc_fail()
{
    struct inode inode;
    struct file file = {0};
    struct simpleFifo_device_data data = {0};

    inode.i_cdev = &data.cdev;

    devm_kzalloc_ExpectAndReturn(data.dev, sizeof(struct file_private_data), GFP_KERNEL, NULL, cmp_pointer, cmp_int, cmp_int);

    int rv = simple_fifo_open(&inode, &file);
    if(rv != -ENOMEM)
    {
        easyMock_addError(easyMock_true, "simple_fifo_open didn't return -ENOMEM (%d)", rv);
    }
    return 0;
}

static void check_result(struct file_private_data* data, uint8_t expectedSize, uint8_t expectedReadOffset, uint8_t expectedWriteOffset, void* bufToExpect)
{
    if(data->size != expectedSize)
    {
        easyMock_addError(easyMock_true, "tested function didn't update size to expectedSize (%d != %d)", data->size, expectedSize);
    }
    if(data->readOffset != expectedReadOffset)
    {
        easyMock_addError(easyMock_true, "tested function didn't update readOffset to expectedReadOffset (%d != %d)", data->readOffset, expectedReadOffset);
    }
    if(data->writeOffset != expectedWriteOffset)
    {
        easyMock_addError(easyMock_true, "tested function didn't update writeOffset to expectedWriteOffset (%d != %d)", data->writeOffset, expectedWriteOffset);
    }
    if(memcmp(data->data, bufToExpect, MAX_FIFO_SIZE) != 0)
    {
        easyMock_addError(easyMock_true, "tested function didn't update data.data correctly. data->data != bufToExpect");
    }
}

static void test_INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static void _test_list_add(struct list_head *new,
                           struct list_head *prev,
                           struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static void test_list_add_tail(struct list_head *new, struct list_head *head)
{
    _test_list_add(new, head->next, head);
}

static void prepare_for_each_files(unsigned int nb_files, unsigned int last_fail)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,46)
    for(unsigned int file_idx = 0; file_idx < nb_files; ++file_idx)
    {
        list_is_head_ExpectAndReturn(NULL, NULL, 0, NULL, NULL);
    }
    if(!last_fail)
    {
        list_is_head_ExpectAndReturn(NULL, NULL, 1, NULL, NULL);
    }
#endif
}

static void prepare_one_file(struct simpleFifo_device_data* dev_data, struct file* file, struct file_private_data* fpd)
{
    test_INIT_LIST_HEAD(&dev_data->opened_file_list);
    fpd->parent = dev_data;
    test_list_add_tail(&fpd->file_entry, &dev_data->opened_file_list);
    file->private_data = (void*)fpd;
}

static void prepare_write_two_file(struct simpleFifo_device_data* dev_data, struct file_private_data* fpd)
{
    test_INIT_LIST_HEAD(&dev_data->opened_file_list);
    for(uint8_t idx = 0; idx < 2; ++idx)
    {
        fpd[idx].parent = dev_data;
        test_list_add_tail(&fpd[idx].file_entry, &dev_data->opened_file_list);
    }
}

int test_simple_fifo_write_simple_write()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);
    prepare_for_each_files(1, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return len (%zd)", len);
    }
    check_result(&fpd, len, 0, len, buf);
    return 0;
}

static int test_write_file(int n)
{
    struct simpleFifo_device_data dev_data;
    struct file_private_data fpd[2] = {{0}, {0}};
    prepare_write_two_file(&dev_data, fpd);

    struct file file = {0};
    file.private_data = &fpd[n];
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(2, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);
    prepare_for_each_files(2, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return len (%zd)", len);
    }
    for(uint8_t idx = 0; idx < 2; ++idx)
    {
        check_result(&fpd[idx], len, 0, len, buf);
    }
    return 0;
}

int test_simple_fifo_write_simple_write_two_files_write_first_file()
{
    return test_write_file(0);
}

int test_simple_fifo_write_simple_write_two_files_write_second_file()
{
    return test_write_file(1);
}

int test_simple_fifo_write_wrapper_write()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    fpd.writeOffset = MAX_FIFO_SIZE - 4;
    fpd.readOffset = fpd.writeOffset;
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, &buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, &buf, len);
    prepare_for_each_files(1, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

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
    check_result(&fpd, len, fpd.readOffset, offsetToExpect, bufToExpect);
    return 0;
}

int test_simple_fifo_write_double_write()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);
    prepare_for_each_files(1, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);
    prepare_for_each_files(1, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

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
    check_result(&fpd, len * 2, 0, len * 2, bufToExpect);
    return 0;
}

int test_simple_fifo_write_copy_from_user_fails()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    fpd.writeOffset = MAX_FIFO_SIZE - 4;
    fpd.readOffset = fpd.writeOffset;

    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, &buf, len, 1, cmp_not_null_pointer, cmp_pointer, cmp_long, &buf, len);

    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != -EFAULT)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return an error");
    }
    check_result(&fpd, fpd.size, fpd.readOffset, fpd.writeOffset, &fpd.data);
    return 0;
}

int test_simple_fifo_write_fifo_full()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    fpd.writeOffset = MAX_FIFO_SIZE - 1;
    fpd.size = MAX_FIFO_SIZE;

    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return 0");
    }
    check_result(&fpd, fpd.size, fpd.readOffset, fpd.writeOffset, &fpd.data);
    return 0;
}

int test_simple_fifo_write_fifo_partial_write()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    fpd.writeOffset = 10;
    fpd.size = MAX_FIFO_SIZE - 4;

    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    // First write
    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, 4, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, 4);
    prepare_for_each_files(1, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    // Second write
    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(1, DO_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != 4)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return partial write of 4 (%zd)", len);
    }
    rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return 0 after partial write");
    }
    uint8_t dataToExpect[MAX_FIFO_SIZE] = {0};
    dataToExpect[10] = 's';
    dataToExpect[11] = 'i';
    dataToExpect[12] = 'm';
    dataToExpect[13] = 'p';
    check_result(&fpd, MAX_FIFO_SIZE, 0, 14, dataToExpect);
    return 0;
}

int test_simple_fifo_write_fifo_write_first_file_second_is_full()
{
    struct simpleFifo_device_data dev_data;
    struct file_private_data fpd[2] = {{0}, {0}};
    prepare_write_two_file(&dev_data, fpd);

    // Write first file
    struct file file = {0};
    file.private_data = &fpd[0];

    // Second is full
    memset(&fpd[1].data, 'a', MAX_FIFO_SIZE);
    fpd[1].writeOffset = MAX_FIFO_SIZE - 1;
    fpd[1].size = MAX_FIFO_SIZE;

    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(2, DO_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return 0");
    }

    // First queue remains empty
    check_result(&fpd[0], 0, 0, 0, &fpd[0].data);

    // First queue remains full
    check_result(&fpd[1], MAX_FIFO_SIZE, 0, MAX_FIFO_SIZE - 1, &fpd[1].data);

    return 0;
}

int test_simple_fifo_write_fifo_write_first_file_second_is_partial_write()
{
    struct simpleFifo_device_data dev_data;
    struct file_private_data fpd[2] = {{0}, {0}};
    prepare_write_two_file(&dev_data, fpd);

    // Write first file
    struct file file = {0};
    file.private_data = &fpd[0];

    // Second is partial
    fpd[1].writeOffset = 10;
    fpd[1].size = MAX_FIFO_SIZE - 4;

    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    // First write
    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(2, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, 4, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, 4);
    prepare_for_each_files(2, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    // Second write
    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(2, DO_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != 4)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return partial write of 4 (%zd)", len);
    }
    rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return 0 after partial write");
    }

    uint8_t file1dataToExpect[MAX_FIFO_SIZE] = {0};
    file1dataToExpect[0] = 's';
    file1dataToExpect[1] = 'i';
    file1dataToExpect[2] = 'm';
    file1dataToExpect[3] = 'p';
    check_result(&fpd[0], 4, 0, 4, file1dataToExpect);

    uint8_t file2dataToExpect[MAX_FIFO_SIZE] = {0};
    file2dataToExpect[10] = 's';
    file2dataToExpect[11] = 'i';
    file2dataToExpect[12] = 'm';
    file2dataToExpect[13] = 'p';
    check_result(&fpd[1], MAX_FIFO_SIZE, 0, 14, file2dataToExpect);
    return 0;
}

int test_simple_fifo_write_fifo_write_two_file_big_data()
{
    struct simpleFifo_device_data dev_data;
    struct file_private_data fpd[2] = {{0}, {0}};
    prepare_write_two_file(&dev_data, fpd);

    // Write first file
    struct file file = {0};
    file.private_data = &fpd[0];

    char buf[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(2, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, MAX_FIFO_SIZE, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, MAX_FIFO_SIZE);
    prepare_for_each_files(2, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != MAX_FIFO_SIZE)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return MAX_FIFO_SIZE != (%zd)", len);
    }

    return 0;
}

int test_simple_fifo_write_fifo_write_two_file_one_is_write_only()
{
    struct simpleFifo_device_data dev_data;
    struct file_private_data fpd[2] = {{0}, {0}};
    prepare_write_two_file(&dev_data, fpd);

    struct file file = {0};
    file.f_flags |= O_WRONLY;
    file.private_data = &fpd[0];
    char buf[MAX_FIFO_SIZE] = "simple char";
    ssize_t len = strlen(buf);
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    prepare_for_each_files(2, DO_NOT_FAIL);
    copy_from_user_ExpectReturnAndOutput(NULL, buf, len, 0, cmp_not_null_pointer, cmp_pointer, cmp_long, buf, len);
    prepare_for_each_files(2, DO_NOT_FAIL);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_write(&file, buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_write didn't return len (%zd)", len);
    }

    char expectedBuf0[MAX_FIFO_SIZE] = {0};
    check_result(&fpd[0], 0, 0, 0, expectedBuf0);
    check_result(&fpd[1], len, 0, len, buf);
    return 0;
}

int test_simple_fifo_read_simple_read()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    char bufToReturn[] = "simple char";
    ssize_t len = strlen(bufToReturn) + 1;
    snprintf((char*)fpd.data, MAX_FIFO_SIZE, "%s", bufToReturn);
    fpd.writeOffset = len;
    fpd.readOffset = fpd.writeOffset - len;
    fpd.size = len;
    char buf = '\0';
    loff_t offset;
    uint8_t expectedReadOffset = fpd.readOffset + fpd.size;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    copy_to_user_ExpectAndReturn(&buf, bufToReturn, len, 0, cmp_pointer, cmp_str, cmp_long);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_read(&file, &buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_read didn't return len (%zd)", len);
    }
    check_result(&fpd, 0, expectedReadOffset, len, &fpd.data);
    return 0;
}

int test_simple_fifo_read_double_read()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    char firstBufToReturn[] = "simple char";
    char secondBufToReturn[] = "and another one";
    ssize_t firstBufLen = strlen(firstBufToReturn) + 1;
    ssize_t secondBufLen = strlen(secondBufToReturn) + 1;
    ssize_t fifoSize = firstBufLen + secondBufLen;
    snprintf((char*)fpd.data, MAX_FIFO_SIZE, "%s%c%s", firstBufToReturn, '\0', secondBufToReturn);
    fpd.writeOffset = fifoSize;
    fpd.readOffset = fpd.writeOffset - fifoSize;
    fpd.size = fifoSize;
    char buf = '\0';
    loff_t offset;
    uint8_t expectedReadOffset = fpd.readOffset + fpd.size;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    copy_to_user_ExpectAndReturn(&buf, firstBufToReturn, firstBufLen, 0, cmp_pointer, cmp_str, cmp_long);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    copy_to_user_ExpectAndReturn(&buf, secondBufToReturn, secondBufLen, 0, cmp_pointer, cmp_str, cmp_long);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_read(&file, &buf, firstBufLen, &offset);
    if(rv != firstBufLen)
    {
        easyMock_addError(easyMock_true, "simple_fifo_read didn't return len on call 1 (%zd)", firstBufLen);
    }
    rv = simple_fifo_read(&file, &buf, secondBufLen, &offset);
    if(rv != secondBufLen)
    {
        easyMock_addError(easyMock_true, "simple_fifo_read didn't return len on call 2(%zd)", secondBufLen);
    }
    check_result(&fpd, 0, expectedReadOffset, fpd.writeOffset, &fpd.data);
    return 0;
}

int test_simple_fifo_read_empty_fifo()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    char buf = '\0';
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_read(&file, &buf, 42, &offset);
    if(rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_read didn't return 0");
    }
    check_result(&fpd, 0, 0, 0, &fpd.data);
    return 0;
}

int test_simple_fifo_read_wrap_read()
{
    char dataBuf[MAX_FIFO_SIZE] = {0};
    dataBuf[60] = 's';
    dataBuf[61] = 'i';
    dataBuf[62] = 'm';
    dataBuf[63] = 'p';
    dataBuf[ 0] = 'l';
    dataBuf[ 1] = 'e';
    dataBuf[ 2] = ' ';
    dataBuf[ 3] = 'c';
    dataBuf[ 4] = 'h';
    dataBuf[ 5] = 'a';
    dataBuf[ 6] = 'r';
    dataBuf[ 7] = '\0';
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    memcpy(fpd.data, dataBuf, MAX_FIFO_SIZE);
    fpd.writeOffset = 8;
    fpd.readOffset = 60;
    fpd.size = 12;

    char bufToExpect[] = "simple char";

    char buf = '\0';
    ssize_t len = strlen(bufToExpect)+1;
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    copy_to_user_ExpectAndReturn(&buf, bufToExpect, len, 0, cmp_pointer, cmp_str, cmp_long);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_read(&file, &buf, len, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_read didn't return len (%zd != %zd)", rv, len);
    }
    check_result(&fpd, 0, 8, 8, dataBuf);
    return 0;
}

int test_simple_fifo_read_request_too_big()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);

    char bufToReturn[] = "simple char";
    ssize_t len = strlen(bufToReturn) + 1;
    snprintf((char*)fpd.data, MAX_FIFO_SIZE, "%s", bufToReturn);
    fpd.writeOffset = len;
    fpd.readOffset = fpd.writeOffset - len;
    fpd.size = len;

    char buf = '\0';
    loff_t offset;
    uint8_t expectedReadOffset = fpd.readOffset + fpd.size;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    copy_to_user_ExpectAndReturn(&buf, bufToReturn, len, 0, cmp_pointer, cmp_str, cmp_long);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_read(&file, &buf, 30, &offset);
    if(rv != len)
    {
        easyMock_addError(easyMock_true, "simple_fifo_read didn't return len (%zd)", len);
    }
    check_result(&fpd, 0, expectedReadOffset, len, &fpd.data);
    return 0;
}

int test_simple_fifo_read_copy_to_user_fails()
{
    struct simpleFifo_device_data dev_data;
    struct file file = {0};
    struct file_private_data fpd = {0};
    prepare_one_file(&dev_data, &file, &fpd);
    char bufToReturn[] = "simple char";
    ssize_t len = strlen(bufToReturn) + 1;
    snprintf((char*)fpd.data, MAX_FIFO_SIZE, "%s", bufToReturn);
    fpd.writeOffset = len;
    fpd.readOffset = fpd.writeOffset - len;
    fpd.size = len;

    char buf = '\0';
    loff_t offset;

    mutex_lock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);
    copy_to_user_ExpectAndReturn(&buf, bufToReturn, len, 1, cmp_pointer, cmp_str, cmp_long);
    mutex_unlock_ExpectAndReturn(&dev_data.open_file_list_mutex, cmp_pointer);

    ssize_t rv = simple_fifo_read(&file, &buf, len, &offset);
    if(rv != -EFAULT)
    {
        easyMock_addError(easyMock_true, "simple_fifo_read didn't return -EFAULT. It returned %ld", rv);
    }
    check_result(&fpd, fpd.size, fpd.readOffset, fpd.writeOffset, &fpd.data);
    return 0;
}

int test_simple_fifo_release()
{
    struct inode inode;
    struct simpleFifo_device_data parent;
    struct file file = {0};
    struct file_private_data fpd = {0};

    fpd.parent = &parent;

    parent.dev = (struct device*)0xdeadbeef;

    mutex_lock_ExpectAndReturn(&parent.open_file_list_mutex, cmp_pointer);
    list_del_ExpectAndReturn(&fpd.file_entry, cmp_pointer);
    devm_kfree_ExpectAndReturn(parent.dev, &fpd, cmp_pointer, cmp_pointer);
    mutex_unlock_ExpectAndReturn(&parent.open_file_list_mutex, cmp_pointer);

    file.private_data = (void*)&fpd;

    int rv = simple_fifo_release(&inode, &file);
    if (rv != 0)
    {
        easyMock_addError(easyMock_true, "simple_fifo_release didn't return 0. %d", rv);
    }
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
    _printk_ExpectAndReturn(NULL, 0, NULL);

    simple_fifo_exit();

    return 0;
}

