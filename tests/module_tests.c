#define module_init(initfn)
#define module_exit(initfn)
#define __init
#define __exit
static struct module{} __this_module;
#define THIS_MODULE (&__this_module)
#include "../simpleFifoModule/simpleFifo.c"

#include <easyMock.h>

#include <stdio.h>
#include <string.h>

static dev_t major_minor_to_test = MKDEV(42, 0);

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
            fprintf(stderr, "simple_fifo_init didn't return 0\n");
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
            fprintf(stderr, "simple_fifo_init didn't return an error\n");
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
            fprintf(stderr, "simple_fifo_init didn't return an error\n");
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
            fprintf(stderr, "simple_fifo_init didn't return an error\n");
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
            fprintf(stderr, "simple_fifo_init didn't return an error\n");
            return 1;
        }
    }
    return 0;
}

int test_exit_module()
{
    struct class* ptr_to_check = (struct class*)0xf00ba4;
    my_class = ptr_to_check;

    expect_device_destroy(ptr_to_check);

    class_destroy_ExpectAndReturn(ptr_to_check, cmp_pointer);

    expect_unregister_chrdev_region();
    printk_ExpectAndReturn(NULL, 0, NULL);

    simple_fifo_exit();

    return 0;
}

