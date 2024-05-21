#!/bin/bash

path=./kernel
build=./build

nasm -felf64 $path/boot.asm -o $build/boot.o
nasm -felf64 $path/multiboot_header.asm -o $build/multiboot_header.o
nasm -felf64 $path/long_mode_start.asm -o $build/long_mode_start.o

$HOME/opt/cross/bin/x86_64-elf-gcc -c $path/string.c -o $build/string.o -std=gnu99 -ffreestanding -Wall -Wextra -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2
$HOME/opt/cross/bin/x86_64-elf-gcc -c $path/vga.c -o $build/vga.o -std=gnu99 -ffreestanding -Wall -Wextra -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2
$HOME/opt/cross/bin/x86_64-elf-gcc -c $path/debug.c -o $build/debug.o -std=gnu99 -ffreestanding -Wall -Wextra -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2
$HOME/opt/cross/bin/x86_64-elf-gcc -c $path/kernel.c -o $build/kernel.o -std=gnu99 -ffreestanding -Wall -Wextra -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 

ld -n -T linker.ld $build/boot.o $build/multiboot_header.o $build/long_mode_start.o $build/vga.o $build/string.o $build/debug.o $build/kernel.o -o MyOS_old.bin -nostdlib -v

cp MyOS.bin iso/boot/
grub-mkrescue -o MyOS.iso iso/
qemu-system-x86_64 -cdrom MyOS.iso 