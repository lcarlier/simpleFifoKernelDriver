cmake_minimum_required (VERSION 3.10)
project (SimpleFifoDevice)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

execute_process(COMMAND uname -r OUTPUT_VARIABLE KERNEL_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

enable_testing()

add_subdirectory(simpleFifoModule)

set(LLVM_DEFAULT_VERSION 12)
set(EASYMOCK_FORCE_BUILD_TYPE Release)
set(ENABLE_ASAN 0)
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/EasyMock/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()
add_subdirectory(extern/EasyMock)
add_subdirectory(tests)