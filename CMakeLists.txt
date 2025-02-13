cmake_minimum_required(VERSION 3.22)

project(ester LANGUAGES C ASM_NASM)

set(ESTER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_COMPILER   /home/manuel/opt/cross/bin/x86_64-elf-gcc) # to be properly paramitrazed

set(GCC_COMPILE_FLAGS "-std=gnu99 -ffreestanding -Wall -Wextra -mno-red-zone -mno-mmx -mno-sse -mno-sse2")
set(GCC_LINK_FLAGS    "-n -nostdlib -v")
set(CMAKE_C_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set(LINKER_SCRIPT ${ESTER_DIR}/kernel/linker.ld)
add_link_options("-T${LINKER_SCRIPT}")

include_directories(
    ${ESTER_DIR}/include
)

include(${ESTER_DIR}/kernel/Filelists.cmake)
add_executable(ester.bin ${ESTER_KERNEL_SRC})   # Just to give to cmake a language ... 

include(${ESTER_DIR}/pci/Filelists.cmake)
include(${ESTER_DIR}/arch/x86_64/Filelists.cmake)
include(${ESTER_DIR}/drivers/Filelists.cmake)
include(${ESTER_DIR}/fs/Filelists.cmake)

# Run qemu adding also a SATA AHCI disk REMEMBER TO CREATE THE DISK IMAGE FIRST
add_custom_target(iso COMMAND cp ester.bin ${ESTER_DIR}/iso/boot/ COMMAND grub-mkrescue -o ester.iso ${ESTER_DIR}/iso/ COMMAND qemu-system-x86_64 -d trace:handle_cmd_* -D qemu-log.txt -m 8G -monitor stdio -cdrom ester.iso -drive file=ext2_example.img,format=raw,if=none,id=drive-sata0 -device ahci,id=ahci -device ide-hd,drive=drive-sata0,bus=ahci.0) # -s -S )

# add_custom_target(iso COMMAND cp ester.bin ${ESTER_DIR}/iso/boot/ COMMAND grub-mkrescue -o ester.iso ${ESTER_DIR}/iso/ COMMAND qemu-system-x86_64 -d trace:ahci*,in_asm -D qemu-log.txt -m 8G -monitor stdio -cdrom ester.iso -hda temp.img)
