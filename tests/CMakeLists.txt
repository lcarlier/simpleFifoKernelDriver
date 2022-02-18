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

# Finally prepend the common arguments
set(KERNEL_COMPILE_COMMAND_ARGS --ignore-parser-error --generate-types -o ${CMAKE_CURRENT_BINARY_DIR} --cwd /lib/modules/${KERNEL_VERSION}/build ${KERNEL_COMPILE_COMMAND_ARGS})

MESSAGE(STATUS "Kernel module compile flag parameters : ${KERNEL_COMPILE_COMMAND_ARGS}")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/linux)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/linux/device)

add_custom_command(OUTPUT easyMock_cdev.c linux/cdev.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/cdev.h
        --generate-comparator-of cdev
        --generate-comparator-of list_head
        --generate-comparator-of kobject
        --generate-comparator-of kref
        --generate-comparator-of refcount_t
        --generate-comparator-of atomic_t
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_cdev.h linux/cdev.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/cdev.h
        EasyMockGenerate
        )

# Giving foobar to mock-only parameter makes sure that non of the function are mocked. As a results, only the macros
# are generated
add_custom_command(OUTPUT easyMock_module.c linux/module.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/module.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_module.h linux/module.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/module.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_kernel.c linux/kernel.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/kernel.h
        --ignore-generation-of vsscanf
        --ignore-generation-of vsprintf
        --ignore-generation-of printf
        --ignore-generation-of snprintf
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_kernel.h linux/kernel.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/kernel.h
        EasyMockGenerate
        EasyMockFramework
        )

add_custom_command(OUTPUT easyMock_fs.c linux/fs.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/fs.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_fs.h linux/fs.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/fs.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_device.c linux/device.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/device.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_device.h linux/device.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/device.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_types.c linux/types.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/types.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_types.h linux/types.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/types.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_uaccess.c linux/uaccess.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/uaccess.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_uaccess.h linux/uaccess.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/uaccess.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_list.c linux/list.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/list.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_list.h linux/list.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/list.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_mutex.c linux/mutex.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/mutex.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_mutex.h linux/mutex.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/mutex.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_printk.c linux/printk.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/printk.h
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../easyMock_printk.h linux/printk.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/printk.h
        EasyMockGenerate
        )

add_custom_command(OUTPUT easyMock_class.c linux/device/class.h
        COMMAND EasyMockGenerate ARGS -i /lib/modules/${KERNEL_VERSION}/build/include/linux/device/class.h
        --generate-comparator-of class
        --generate-attribute format
        ${KERNEL_COMPILE_COMMAND_ARGS}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ../../easyMock_class.h linux/device/class.h
        DEPENDS
        /lib/modules/${KERNEL_VERSION}/build/include/linux/device/class.h
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
        easyMock_list.c
        easyMock_mutex.c
        easyMock_printk.c
        easyMock_class.c
        module_tests.c
        main.cpp
        )
target_compile_options(testKernel PRIVATE -g -O0 -Wall -Werror -Wextra -Wno-unused)

target_include_directories(testKernel PRIVATE ${PROJECT_SOURCE_DIR}/extern/EasyMock/src/easyMockFramework/include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(testKernel EasyMockFramework)

add_test(NAME testKernel COMMAND testKernel)
