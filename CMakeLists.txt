cmake_minimum_required(VERSION 3.30)
include(cmake/ArmToolchain.cmake)

project(dual-elevator-spi C)

set(CMAKE_C_STANDARD                11)
set(CMAKE_C_STANDARD_REQUIRED       ON)
set(CMAKE_C_EXTENSIONS              ON)
enable_language(ASM)

# Build role: MASTER generates Board A hex, SLAVE generates Board B hex.
# Example:
#   cmake -S . -B build-master -GNinja -DELEVATOR_ROLE=MASTER
#   cmake -S . -B build-slave  -GNinja -DELEVATOR_ROLE=SLAVE
set(ELEVATOR_ROLE "MASTER" CACHE STRING "MASTER or SLAVE")
set_property(CACHE ELEVATOR_ROLE PROPERTY STRINGS MASTER SLAVE)

set(PROJECT_PATH                         ${CMAKE_CURRENT_SOURCE_DIR})
include(cmake/Device.cmake)
include(cmake/SourceList.cmake)
include(cmake/IncludeList.cmake)

if(ELEVATOR_ROLE STREQUAL "MASTER")
    # Removes ANY file ending in "_Slave.c" (App_Slave.c, Spi_Slave.c, etc.)
    list(FILTER SOURCE_LIST EXCLUDE REGEX ".*_Slave\\.c$")
    
elseif(ELEVATOR_ROLE STREQUAL "SLAVE")
    # Removes ANY file ending in "_Master.c" (App_Master.c, Spi_Master.c, etc.)
    list(FILTER SOURCE_LIST EXCLUDE REGEX ".*_Master\\.c$")
endif()

set(LINKER_SCRIPTS_BASE_DIR              ${PROJECT_PATH}/STM32-base/linker/)
set(STARTUP_BASE_DIR                     ${PROJECT_PATH}/STM32-base/startup/)

set(EXECUTABLE                           ${CMAKE_PROJECT_NAME})

# Executable files
add_executable(${EXECUTABLE} ${SOURCE_LIST} ${STARTUP_BASE_DIR}/${SERIES_FOLDER}/${MAPPED_DEVICE}.s)
# Include
target_include_directories(${EXECUTABLE} PUBLIC ${INCLUDE_LIST})
# Project symbols
target_compile_definitions(${EXECUTABLE} PUBLIC ${COMPILER_DEFINE} ELEVATOR_ROLE_${ELEVATOR_ROLE})
# Compiler options
target_compile_options(${EXECUTABLE} PUBLIC
        ${CPU_PARAMETERS}
        -Wall
        -Wpedantic
        -Wno-unused-parameter
)

# Linker options
target_link_options(${EXECUTABLE} PRIVATE
        ${CPU_PARAMETERS}
        -L${LINKER_SCRIPTS_BASE_DIR}
        -T${LINKER_SCRIPTS_BASE_DIR}/${SERIES_FOLDER}/${DEVICE}.ld
#        -nostartfiles
        -Xlinker --gc-sections
        -Wl,-Map=${CMAKE_PROJECT_NAME}.map
        --specs=nano.specs
        #-u _printf_float                # STDIO float formatting support
)

# Execute post-build to print size
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>
)
# Convert output to hex and binary
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.hex
)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.bin
)
