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

TEST_CASE("init module", "[init_module]")
{
    initialise_easyMock();
    SECTION("Module init no error")
    {
        CHECK(test_init_module_no_errors() == 0);
    }
    SECTION("alloc_chrdev_region fails")
    {
        CHECK(test_init_module_alloc_chrdev_region_fail() == 0);
    }
    SECTION("lass_create fails")
    {
        CHECK(test_init_module_class_create_fail() == 0);
    }
    SECTION("cdev_add fails")
    {
        CHECK(test_init_module_cdev_add_fail() == 0);
    }
    SECTION("device_create fails")
    {
        CHECK(test_init_module_device_create_fail() == 0);

    }
    check_easyMock();
}

TEST_CASE("open file", "[open]")
{
    initialise_easyMock();
    SECTION("Open simple fifo")
    {
        CHECK(test_simple_fifo_open() == 0);
    }
    check_easyMock();
}


TEST_CASE("module exit", "[module_exit]")
{
    initialise_easyMock();
    SECTION("module exit OK")
    {
        REQUIRE(test_exit_module() == 0);
    }
    check_easyMock();
}