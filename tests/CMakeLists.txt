# Magic command to extract all the compile parameters given when compiling with clang
execute_process(COMMAND bash -c "make -n -C /lib/modules/${KERNEL_VERSION}/build M=${PROJECT_SOURCE_DIR}/simpleFifoModule CC=clang-12 V=1 VERBOSE=1 modules | sed -n -e 's/^.*clang-12 \\(.*\\) -c -o.*$/\\1/p'"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/simpleFifoModule
        ERROR_QUIET
        COMMAND_ECHO STDOUT
        OUTPUT_VARIABLE KERNEL_COMPILE_COMMAND_ARGS
        OUTPUT_STRIP_TRAILING_WHITESPACE)

# First remove garbage
string(REPLACE "\\" "" KERNEL_COMPILE_COMMAND_ARGS ${KERNEL_COMPILE_COMMAND_ARGS})
string(REPLACE "'''" "'" KERNEL_COMPILE_COMMAND_ARGS ${KERNEL_COMPILE_COMMAND_ARGS})

# Then transform to a list
string(REPLACE " " ";" KERNEL_COMPILE_COMMAND_ARGS ${KERNEL_COMPILE_COMMAND_ARGS})

#Finally prepend the common arguments
set(KERNEL_COMPILE_COMMAND_ARGS --generate-types -o ${CMAKE_CURRENT_BINARY_DIR} --cwd /lib/modules/${KERNEL_VERSION}/build ${KERNEL_COMPILE_COMMAND_ARGS})

MESSAGE(STATUS "Kernel module compile flag parameters : ${KERNEL_COMPILE_COMMAND_ARGS}")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/linux)

add_custom_command(OUTPUT easyMock_cdev.c linux/cdev.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/cdev.h --mock-only cdev_alloc --mock-only cdev_init --mock-only cdev_add --mock-only cdev_del --mock-only class_unregister ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_cdev.h linux/cdev.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/cdev.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_module.c linux/module.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/module.h --mock-only foobar ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_module.h linux/module.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/module.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_kernel.c linux/kernel.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/kernel.h --mock-only printk ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_kernel.h linux/kernel.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/kernel.h
        EasyMockGenerate
        EasyMockFramework
        )

add_custom_command(OUTPUT easyMock_fs.c linux/fs.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/fs.h --mock-only register_chrdev_region --mock-only alloc_chrdev_region --mock-only register_chrdev --mock-only unregister_chrdev_region ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_fs.h linux/fs.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/fs.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_device.c linux/device.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/device.h --mock-only __class_create --mock-only device_create --mock-only device_destroy --mock-only class_unregister --mock-only class_destroy --generate-attribute format ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_device.h linux/device.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/device.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_types.c linux/types.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/types.h --generate-attribute format ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_types.h linux/types.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/types.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_uaccess.c linux/uaccess.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/uaccess.h --mock-only copy_from_user --mock-only copy_to_user --generate-attribute format ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_uaccess.h linux/uaccess.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/uaccess.h
        EasyMockGenerate
        )

add_executable(testKernel
        easyMock_cdev.c
        easyMock_module.c
        easyMock_kernel.c
        easyMock_fs.c
        easyMock_device.c
        easyMock_types.c
        easyMock_uaccess.c
        module_tests.c
        main.cpp
        )
target_compile_options(testKernel PRIVATE -g -O0 -Wall -Werror -Wextra -Wno-unused)

target_include_directories(testKernel PRIVATE ${PROJECT_SOURCE_DIR}/extern/EasyMock/src/easyMockFramework/include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(testKernel EasyMockFramework)
target_link_options(testKernel PRIVATE -Wl,--allow-multiple-definition)

add_test(NAME testKernel COMMAND testKernel)
