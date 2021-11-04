#ifndef SIMPLEFIFODEVICE_MODULE_TESTS_H
#define SIMPLEFIFODEVICE_MODULE_TESTS_H

#ifdef __cplusplus
extern "C"
{
#endif

    int test_init_module_no_errors();
    int test_init_module_alloc_chrdev_region_fail();
    int test_init_module_class_create_fail();
    int test_init_module_cdev_add_fail();
    int test_init_module_device_create_fail();

    int test_simple_fifo_open();

    int test_exit_module();

#ifdef __cplusplus
};
#endif

#endif //SIMPLEFIFODEVICE_MODULE_TESTS_H
