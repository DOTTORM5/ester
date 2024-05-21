cmake_minimum_required(VERSION 3.22)

project(ester LANGUAGES C ASM_NASM)

set(ESTER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_COMPILER   /home/manuel/opt/cross/bin/x86_64-elf-gcc) # to be properly paramitrazed

# set(GCC_COMPILE_FLAGS "-std=gnu99 -ffreestanding -Wall -Wextra -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2")
set(GCC_COMPILE_FLAGS "-std=gnu99 -ffreestanding -Wall -Wextra -mno-red-zone -mno-mmx -mno-sse -mno-sse2")
set(GCC_LINK_FLAGS    "-n -nostdlib -v")
set(CMAKE_C_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set(LINKER_SCRIPT ${ESTER_DIR}/kernel/linker.ld)
add_link_options("-T${LINKER_SCRIPT}")

set (ESTER_INCLUDE_DIRS
    "${ESTER_DIR}/include"
)

include(${ESTER_DIR}/kernel/Filelists.cmake)
include(${ESTER_DIR}/pci/Filelists.cmake)

add_executable(ester.bin ${ESTER_KERNEL_SRC} ${ESTER_PCI_SRC} ${ESTER_KERNEL_ASM} ${LINKER_SCRIPT})
target_include_directories(ester.bin PRIVATE ${ESTER_INCLUDE_DIRS})
set_target_properties(ester.bin PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

add_custom_target(iso COMMAND cp ester.bin ${ESTER_DIR}/iso/boot/ COMMAND grub-mkrescue -o ester.iso ${ESTER_DIR}/iso/ COMMAND qemu-system-x86_64 -cdrom ester.iso -s -S )