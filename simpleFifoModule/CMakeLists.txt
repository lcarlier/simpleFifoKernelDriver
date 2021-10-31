set(LINUX_HEADER_DIR "/lib/modules/${KERNEL_VERSION}" CACHE PATH "Path to the linux headers file")
set(LINUX_HEADER_BUILD_DIR "${LINUX_HEADER_DIR}/build")
if(NOT EXISTS "${LINUX_HEADER_BUILD_DIR}")
    message(FATAL_ERROR "Linux kernel Makefile not found not found at ${LINUX_HEADER_BUILD_DIR}. Please install linux headers.")
endif()

set(MODULE_PATH ${PROJECT_SOURCE_DIR}/simpleFifoModule)

add_custom_command(OUTPUT simpleFifo.ko
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/* ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND make ARGS -C ${LINUX_HEADER_BUILD_DIR} M=${CMAKE_CURRENT_BINARY_DIR} modules
        DEPENDS simpleFifo.c)

add_custom_target(kernel-module ALL
        DEPENDS simpleFifo.ko)

