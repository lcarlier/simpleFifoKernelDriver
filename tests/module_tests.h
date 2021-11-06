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
    int test_simple_fifo_open_devm_kzalloc_fail();

    int test_simple_fifo_write_simple_write();
    int test_simple_fifo_write_simple_write_two_files_write_first_file();
    int test_simple_fifo_write_simple_write_two_files_write_second_file();
    int test_simple_fifo_write_wrapper_write();
    int test_simple_fifo_write_double_write();
    int test_simple_fifo_write_copy_from_user_fails();
    int test_simple_fifo_write_fifo_full();
    int test_simple_fifo_write_fifo_partial_write();
    int test_simple_fifo_write_fifo_write_first_file_second_is_full();
    int test_simple_fifo_write_fifo_write_first_file_second_is_partial_write();
    int test_simple_fifo_write_fifo_write_two_file_big_data();
    int test_simple_fifo_write_fifo_write_two_file_one_is_write_only();

    int test_simple_fifo_read_simple_read();
    int test_simple_fifo_read_double_read();
    int test_simple_fifo_read_empty_fifo();
    int test_simple_fifo_read_wrap_read();
    int test_simple_fifo_read_request_too_big();
    int test_simple_fifo_read_copy_to_user_fails();

    int test_simple_fifo_release();

    int test_exit_module();

#ifdef __cplusplus
};
#endif

#endif //SIMPLEFIFODEVICE_MODULE_TESTS_H
