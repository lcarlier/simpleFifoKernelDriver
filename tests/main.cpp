#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "module_tests.h"

#include <easyMock.h>

namespace
{
    void initialise_easyMock()
    {
        easyMock_init();
        easyMock_setCheckCallsOrder(easyMock_true);
    }

    void check_easyMock()
    {
        if(!easyMock_check())
        {
            INFO(easyMock_getErrorStr());
            CHECK(false);
        }
    }
}

TEST_CASE("Init module", "[init_module]")
{
    initialise_easyMock();
    SECTION("Module init no error")
    {
        CHECK(test_init_module_no_errors() == 0);
        check_easyMock();
    }
    SECTION("Alloc_chrdev_region fails")
    {
        CHECK(test_init_module_alloc_chrdev_region_fail() == 0);
        check_easyMock();
    }
    SECTION("Class_create fails")
    {
        CHECK(test_init_module_class_create_fail() == 0);
        check_easyMock();
    }
    SECTION("Cdev_add fails")
    {
        CHECK(test_init_module_cdev_add_fail() == 0);
        check_easyMock();
    }
    SECTION("Device_create fails")
    {
        CHECK(test_init_module_device_create_fail() == 0);
        check_easyMock();
    }
}

TEST_CASE("Open file", "[open]")
{
    initialise_easyMock();
    SECTION("Open simple fifo")
    {
        CHECK(test_simple_fifo_open() == 0);
        check_easyMock();
    }
    SECTION("Devm_kzalloc fails")
    {
        CHECK(test_simple_fifo_open_devm_kzalloc_fail() == 0);
        check_easyMock();
    }
}


TEST_CASE("Write file", "[write]")
{
    initialise_easyMock();
    SECTION("Simple write")
    {
        CHECK(test_simple_fifo_write_simple_write() == 0);
        check_easyMock();
    }
    SECTION("Simple write first of 2 files")
    {
        CHECK(test_simple_fifo_write_simple_write_two_files_write_first_file() == 0);
        check_easyMock();
    }
    SECTION("Simple write second of 2 files")
    {
        CHECK(test_simple_fifo_write_simple_write_two_files_write_second_file() == 0);
    }
    SECTION("Wrapper write")
    {
        CHECK(test_simple_fifo_write_wrapper_write() == 0);
        check_easyMock();
    }
    SECTION("Double write")
    {
        CHECK(test_simple_fifo_write_double_write() == 0);
        check_easyMock();
    }
    SECTION("Copy from user fails")
    {
        CHECK(test_simple_fifo_write_copy_from_user_fails() == 0);
        check_easyMock();
    }
    SECTION("Fifo full")
    {
        CHECK(test_simple_fifo_write_fifo_full() == 0);
        check_easyMock();
    }
    SECTION("Partial write")
    {
        CHECK(test_simple_fifo_write_fifo_partial_write() == 0);
        check_easyMock();
    }
    SECTION("Write first file but second is full")
    {
        CHECK(test_simple_fifo_write_fifo_write_first_file_second_is_full() == 0);
        check_easyMock();
    }
    SECTION("Write first file but second is partial write")
    {
        CHECK(test_simple_fifo_write_fifo_write_first_file_second_is_partial_write() == 0);
        check_easyMock();
    }
    SECTION("Write two files big data")
    {
        CHECK(test_simple_fifo_write_fifo_write_two_file_big_data() == 0);
        check_easyMock();
    }
    SECTION("Write two files one is write only")
    {
        CHECK(test_simple_fifo_write_fifo_write_two_file_one_is_write_only() == 0);
        check_easyMock();
    }
}

TEST_CASE("Release file", "[release_file]")
{
    initialise_easyMock();
    SECTION("Free private data")
    {
        CHECK(test_simple_fifo_release() == 0);
        check_easyMock();
    }
}

TEST_CASE("Read file", "[read_file]")
{
    initialise_easyMock();
    SECTION("Simple read")
    {
        CHECK(test_simple_fifo_read_simple_read() == 0);
        check_easyMock();
    }
    SECTION("Double read")
    {
        CHECK(test_simple_fifo_read_double_read() == 0);
        check_easyMock();
    }
    SECTION("Empty fifo")
    {
        CHECK(test_simple_fifo_read_empty_fifo() == 0);
        check_easyMock();
    }
    SECTION("Wrap read")
    {
        CHECK(test_simple_fifo_read_wrap_read() == 0);
        check_easyMock();
    }
    SECTION("Read request too big")
    {
        CHECK(test_simple_fifo_read_request_too_big() == 0);
        check_easyMock();
    }
    SECTION("Copy to user fails")
    {
        CHECK(test_simple_fifo_read_copy_to_user_fails() == 0);
        check_easyMock();
    }
}

TEST_CASE("Exit module", "[exit_module]")
{
    initialise_easyMock();
    SECTION("module exit OK")
    {
        CHECK(test_exit_module() == 0);
        check_easyMock();
    }
}